#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include <cJSON.h>

#define HOST_IP_ADDR CONFIG_DEST_IPV4_ADDRESS
#define PORT CONFIG_DEST_PORT

#define DATA_LENGTH 110
#define TCP_SLEEPING 0x00
#define TCP_SENDING 0x01

static const char *TCP_TAG = "example";
static const char *payload = "Message from ESP32 ";

extern SemaphoreHandle_t uneo_data_mutex;
extern MessageBufferHandle_t uneo_data_message;

extern SemaphoreHandle_t uneo_error_mutex;
extern MessageBufferHandle_t uneo_error_message;

extern SemaphoreHandle_t wifi_mutex;
extern uint8_t *uneo_data;

bool tcp_sending_state = TCP_SENDING;

static void tcp_client_task(void *pvParameters)
{
    // Wait until wifi configuration done
    xSemaphoreTake(wifi_mutex, portMAX_DELAY);

    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    uint8_t *data = (uint8_t *)malloc(DATA_LENGTH * 3);
    // char *rx_buffer = (char*) malloc(sizeof(char) * 128);

    while (1)
    {
        ESP_LOGI(TCP_TAG, "Socket creating, connecting to %s:%d", host_ip, PORT);
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            if (sock != -1)
            {
                ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }
            continue;
        }
        ESP_LOGI(TCP_TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 100000;
        int pass;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &pass, sizeof(pass));
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        if (err != 0)
        {
            ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            if (sock != -1)
            {
                ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }
            continue;
        }
        else
        {
            cJSON *register_json;
            register_json = cJSON_CreateObject();
            cJSON_AddStringToObject(register_json, "command", "register");
            cJSON_AddNumberToObject(register_json, "id", CONFIG_UNEO_SENSOR_ID);
            char *start_flag = "STARTOFJSON";
            char *message_s = cJSON_Print(register_json);
            char *end_flag = "ENDOFJSON";
            char message_with_flag[strlen(start_flag) + strlen(message_s) + strlen(end_flag) + 1];
            strcpy(message_with_flag, start_flag);
            strcat(message_with_flag, message_s);
            strcat(message_with_flag, end_flag);

            int err_socket = sendto(sock, message_with_flag, strlen(message_with_flag), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            // int err_socket = sendto(sock, data, DATA_LENGTH * 3, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err_socket < 0)
            {
                ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            cJSON_Delete(register_json);
            cJSON_free(message_s);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TCP_TAG, "Successfully connected");

        while (1)
        {
            if (tcp_sending_state == TCP_SENDING)
            {
                size_t xBytesRead = xMessageBufferReceive(uneo_data_message, data, DATA_LENGTH * 3, portMAX_DELAY);
                if (xBytesRead == DATA_LENGTH * 3)
                {
                    ESP_LOGI(TCP_TAG, "new package");
                    cJSON *root;
                    root = cJSON_CreateObject();
                    cJSON_AddStringToObject(root, "command", "streaming");
                    cJSON_AddNumberToObject(root, "id", CONFIG_UNEO_SENSOR_ID);

                    // -- Fail on sending chars with 330 length
                    // -- JSON can't parser it.
                    // char message_c[DATA_LENGTH * 3+1];
                    // memcpy(message_c, data, DATA_LENGTH * 3);
                    // message_c[DATA_LENGTH * 3+1] = '\0';
                    // cJSON_AddStringToObject(root, "data", message_c);
                    // printf("%s", message_c);

                    cJSON *array, *element;
                    array = cJSON_AddArrayToObject(root, "data");
                    for (int loop = 0; loop < DATA_LENGTH * 3; loop++)
                    {
                        element = cJSON_CreateNumber(data[loop]);
                        cJSON_AddItemToArray(array, element);
                    }
                    char *message_s = cJSON_Print(root);
                    char *start_flag = "STARTOFJSON";
                    char *end_flag = "ENDOFJSON";
                    char data_message_with_flag[strlen(start_flag) + strlen(message_s) + strlen(end_flag) + 1];
                    strcpy(data_message_with_flag, start_flag);
                    strcat(data_message_with_flag, message_s);
                    strcat(data_message_with_flag, end_flag);
                    int err_socket = sendto(sock, data_message_with_flag, strlen(data_message_with_flag), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    // int err_socket = sendto(sock, data, DATA_LENGTH * 3, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err_socket < 0)
                    {
                        ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }

                    cJSON_Delete(root);
                    cJSON_free(message_s);
                }
            }else{
                char* message = "s";
                int err_socket = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                // int err_socket = sendto(sock, data, DATA_LENGTH * 3, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err_socket < 0)
                {
                    ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            ESP_LOGI(TCP_TAG, "rec%d", len);

            if (len == 0)
            {
                ESP_LOGE(TCP_TAG, "Received %d bytes from %s, reconnecting...", len, host_ip);
                break;
            }
            else if (len > 0)
            {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TCP_TAG, "rec%s", rx_buffer);
                cJSON *received_json = cJSON_Parse(rx_buffer);
                if (received_json)
                {
                    ESP_LOGI(TCP_TAG, "succ");
                    cJSON *parseId = cJSON_GetObjectItem(received_json, "id");
                    cJSON *parseAction = cJSON_GetObjectItem(received_json, "command");
                    ESP_LOGI(TCP_TAG, "succ%s", parseAction->valuestring);
                    if (cJSON_IsString(parseAction))
                    {

                        ESP_LOGI(TCP_TAG, "succ%s, %d", parseAction->valuestring, strncmp(parseAction->valuestring, "OFF", 3));

                        if (strncmp(parseAction->valuestring, "OFF", 3) == 0)
                        {
                            tcp_sending_state = TCP_SLEEPING;
                            ESP_LOGI(TCP_TAG, "off match");
                        }
                        else if (strncmp(parseAction->valuestring, "ON", 2) == 0)
                        {
                            tcp_sending_state = TCP_SENDING;
                            ESP_LOGI(TCP_TAG, "on match");
                        }
                    }
                }
            }
            // else
            // {
            //     ESP_LOGE(TCP_TAG, "Error occurred during recving: errno %d", errno);
            //     // ESP_LOGI(TCP_TAG, "Where am i %s", rx_buffer);
            // }
            // vTaskDelay(30 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
