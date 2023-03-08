/*
    This file is for WiFi setting function call.
    Inculding initialize, power save
*/

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_pm.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/*set the ssid and password via "idf.py menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PWD CONFIG_WIFI_PASSWORD

#define DEFAULT_LISTEN_INTERVAL CONFIG_WIFI_LISTEN_INTERVAL

#if CONFIG_POWER_SAVE_MIN_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_POWER_SAVE_MAX_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MAX_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/

extern SemaphoreHandle_t wifi_mutex;

static const char *WIFI_TAG = "WIFI";

xSemaphoreHandle connectionSemaphore;


//Manejador de eventos - event task
//https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    ESP_LOGI(WIFI_TAG,"connecting...\n");
    break;

  case SYSTEM_EVENT_STA_CONNECTED:
    ESP_LOGI(WIFI_TAG,"connected\n");
    break;

  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(WIFI_TAG,"got ip\n");
    xSemaphoreGive(connectionSemaphore);
    xSemaphoreGive(wifi_mutex);
    break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
    ESP_LOGI(WIFI_TAG,"disconnected\n");
    esp_wifi_connect();
    break;

  default:
    break;
  }
  return ESP_OK;
}

/*init wifi as sta and set power save mode*/
static void wifi_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL)); 

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
            .listen_interval = DEFAULT_LISTEN_INTERVAL,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "esp_wifi_set_ps().");
    esp_wifi_set_ps(DEFAULT_PS_MODE);
}

void wifi_connection(void *para)
{
  while (true)
  {
    if (xSemaphoreTake(connectionSemaphore, 10000 / portTICK_RATE_MS))
    {
      ESP_LOGI(WIFI_TAG, "connected");
      // do something
      tcpip_adapter_ip_info_t ip_info;
      ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
      printf("IP Addresss: %s\n", ip4addr_ntoa(&ip_info.ip));
      printf("Subnet Mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
      printf("Gateway: %s\n", ip4addr_ntoa(&ip_info.ip));

      xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
    }
    else
    {
      ESP_LOGE(WIFI_TAG, "Failed to connect. Retry in");
      for (int i = 0; i < 5; i++)
      {
        ESP_LOGE(WIFI_TAG, "...%d", i);
        vTaskDelay(1000 / portTICK_RATE_MS);
      }
    //   esp_wifi_connect();
      esp_restart(); // si no hay conexion reiniciar el chip.
    }
  }
}