#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "freertos/semphr.h"

#define STORAGE_SIZE_BYTES 350

// For uneo
SemaphoreHandle_t uneo_data_mutex = NULL;
SemaphoreHandle_t uneo_error_mutex = NULL;

MessageBufferHandle_t uneo_data_message = NULL;
MessageBufferHandle_t uneo_error_message = NULL;

static uint8_t uneoDataStorageBuffer[ STORAGE_SIZE_BYTES ];
StaticMessageBuffer_t xMessageBufferStruct;
SemaphoreHandle_t udp_sending_mutex = NULL;

SemaphoreHandle_t wifi_mutex = NULL;