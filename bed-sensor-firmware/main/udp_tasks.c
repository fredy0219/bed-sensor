#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_heap_trace.h"

#define HOST_IP_ADDR CONFIG_DEST_IPV4_ADDRESS
#define PORT CONFIG_DEST_PORT

#define UDP_SLEEPING 0x00
#define UDP_SENDING 0x01

#define DATA_LENGTH                 110

extern MessageBufferHandle_t uneo_data_message;

extern SemaphoreHandle_t uneo_error_mutex;
extern MessageBufferHandle_t uneo_error_message;

static const char *UDP_TAG = "UDP";

bool udp_sending_state = UDP_SLEEPING;

UBaseType_t uxHighWaterMarkUDPSend;

static void udp_recv(void *pvParameters){
    static const char *udp_recv_id = "UDP_RECV"; 

    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(UDP_TAG, "%s Socket created, sending to %s:%d",udp_recv_id, HOST_IP_ADDR, PORT);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    bool isSent = false;
    while(!isSent){
        int err_socket = sendto(sock, udp_recv_id, strlen(udp_recv_id), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err_socket < 0) {
            ESP_LOGE(UDP_TAG, "RECV Send fail.");
        }
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
        if (strncmp(rx_buffer, "hihi", 4) == 0) {
            isSent = true;
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }


    while(1){
         // Receiving 
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
        // Error occurred during receiving
        // if (len < 0) {
        //     ESP_LOGE(UDP_TAG, "recvfrom failed: errno %d", errno);
        // }
        if(len > 0){
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(UDP_TAG, "Received %d bytes from %s:", len, host_ip);
            ESP_LOGI(UDP_TAG, "%s", rx_buffer);
            if (strncmp(rx_buffer, "open", 4) == 0) {
                udp_sending_state = UDP_SENDING;
                ESP_LOGI(UDP_TAG, "open match");
            }else if(strncmp(rx_buffer, "close", 5) == 0){
                udp_sending_state = UDP_SLEEPING;
                ESP_LOGI(UDP_TAG, "close match");
            }
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }

}

static void udp_send(void *pvParameters){


    UBaseType_t lastUxHighWaterMarkUDPSend; 
    lastUxHighWaterMarkUDPSend = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGI(UDP_TAG, "uneo_read %d", uxHighWaterMarkUDPSend);

    static const char *udp_send_id = "UDP_SEND"; 

    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    uint8_t *data = (uint8_t *)malloc(DATA_LENGTH * 3);

    uint8_t count = 0;

    while(1){

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
        free(data);
        vTaskDelete(NULL);
    }
    ESP_LOGI(UDP_TAG, "%s Socket created, sending to %s:%d",udp_send_id, HOST_IP_ADDR, PORT);

        while(1){
            
            // ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );

            // uxHighWaterMarkUDPSend = uxTaskGetStackHighWaterMark( NULL );
            // ESP_LOGI(TAG, "uneo_read %d", uxHighWaterMarkUDPSend);
            // Sending
            if(udp_sending_state == UDP_SENDING){
                size_t xBytesRead = xMessageBufferReceive(uneo_data_message, data, DATA_LENGTH * 3, portMAX_DELAY);
                if(xBytesRead == DATA_LENGTH * 3){
                    int err_socket = sendto(sock, data, DATA_LENGTH * 3, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err_socket < 0) {
                        ESP_LOGE(UDP_TAG, "Error occurred during sending: errno %d", errno);
                        esp_restart();
                        break;
                    }else{
                    }
                }else{
                    ESP_LOGE(UDP_TAG, "what wrong? %d", xBytesRead);
                }
            }
            // ESP_ERROR_CHECK( heap_trace_stop() );
            // heap_trace_dump();

            uxHighWaterMarkUDPSend = uxTaskGetStackHighWaterMark( NULL );
            if(lastUxHighWaterMarkUDPSend != uxHighWaterMarkUDPSend){
                ESP_LOGI(UDP_TAG, "uneo_read %d", uxHighWaterMarkUDPSend);
                lastUxHighWaterMarkUDPSend = uxHighWaterMarkUDPSend;
            }

            vTaskDelay(30 / portTICK_PERIOD_MS);


        }

        if (sock != -1) {
            ESP_LOGE(UDP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
}