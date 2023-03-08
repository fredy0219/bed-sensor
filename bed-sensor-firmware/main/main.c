#include <stdio.h>
#include "uneo_i2c.c"
#include "wifi_configuration.c"
#include "udp_tasks.c"
#include "tcp_tasks.c"
#include "semaphore_manager.c"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#define NUM_RECORDS 100
#define DATA_LENGTH 110
#define STORAGE_SIZE_BYTES 1000

#define STATS_TASK_PRIO     3
#define STATS_TICKS         pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET   5

static heap_trace_record_t trace_record[NUM_RECORDS];
static char task_names[4][configMAX_TASK_NAME_LEN];
static SemaphoreHandle_t sync_stats_task;

void wifi_error_send(void *pvParameters){
    uint8_t uneo_err = 0;
    // uint8_t *uneo_err = (uint8_t *)malloc(10);
    uint8_t err;

    while(1){
        err = xMessageBufferReceive(uneo_error_message, &uneo_err, sizeof(uneo_err), portMAX_DELAY);
        xSemaphoreTake(uneo_error_mutex, portMAX_DELAY);
        if(err > 0){
            // printf("*******************\n");
            // for(int i = 0 ; i < 10 ; i ++)
            //     printf("%d ", uneo_err[i]);
            // printf("*******************\n"); 
            printf("UNEO Error: %d \n", uneo_err);
        }
        xSemaphoreGive(uneo_error_mutex);
    }

}

void app_main(void)
{
        //Wifi setup
    connectionSemaphore = xSemaphoreCreateBinary();
    sync_stats_task = xSemaphoreCreateBinary();
    wifi_mutex = xSemaphoreCreateBinary();
    wifi_init();
    xTaskCreate(wifi_connection, "handel comms", 1024 * 3, NULL, 5, NULL);

    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
    //  * Read "Establishing Wi-Fi or Ethernet Connection" section in
    //  * examples/protocols/README.md for more information about this function.
    //  */
    // ESP_ERROR_CHECK(example_connect());
    
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(UNEO_TAG, "I2C initialized successfully");

    uneo_data_message = xMessageBufferCreateStatic(sizeof(uneoDataStorageBuffer), uneoDataStorageBuffer, &xMessageBufferStruct);
    uneo_error_message = xMessageBufferCreate(10);

    xTaskCreate(uneo_read_task, "uneo_read_task", 1024 * 4, (void*)0, 10, NULL);
    // xTaskCreate(udp_recv, "udp_recv", 1024 * 2, (void*)0, 5, NULL);
    // xTaskCreate(udp_send, "udp_send", 1024 * 4, (void*)0, 10, NULL);
    xTaskCreate(tcp_client_task, "tcp_client_task", 1024 * 4, (void*)0, 11, NULL);

}
