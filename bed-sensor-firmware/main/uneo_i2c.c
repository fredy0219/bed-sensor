/*

*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"

static const char *UNEO_TAG = "uneo-i2c";

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                      /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define DELAY_TIME_BETWEEN_ITEMS_MS 200
#define DATA_LENGTH                 110
#define UNEO_PANEL_1_ADDR           CONFIG_UNEO_PANEL_1_ADDR
#define UNEO_PANEL_2_ADDR           CONFIG_UNEO_PANEL_2_ADDR
#define UNEO_PANEL_3_ADDR           CONFIG_UNEO_PANEL_3_ADDR
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1      

#define UNEO_ERROR_TIMEOUT 0x01
#define UNEO_ERROR_PREFIX_FAIL 0x02
#define UNEO_ERROR_NO_ACK 0x04

extern MessageBufferHandle_t uneo_data_message;
extern MessageBufferHandle_t uneo_error_message;

UBaseType_t uxHighWaterMarkI2C;
static esp_err_t uneo_read(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data){
    
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    vTaskDelay(5 / portTICK_PERIOD_MS);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slave_addr << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, DATA_LENGTH-1, ACK_VAL);
    i2c_master_read_byte(cmd, data + DATA_LENGTH-1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static bool uneo_data_check(uint8_t slave_addr, uint8_t *data){
    
    bool check = true;
    uint8_t prefix[6] = {slave_addr, 0x4E, 0x45, 0xFF,0xFF, 0xFF};
    uint8_t postfix[4] = {slave_addr, 0x4E, 0x45, 0xFF};
    
    for(int i = 0 ; i < 6 ; i++){
        if(data[i] != prefix[i]){
            check = false;
        }
    }
    for(int i = DATA_LENGTH - 4 ; i < DATA_LENGTH ; i++){
        if(data[i] != postfix[i - (DATA_LENGTH-4)]){
            check = false;
        }
    }

    // printf("*******************\n");
    // for(int loop = 0; loop < DATA_LENGTH * 3; loop++)
    //     printf("%d ", data[loop]);
    // printf("*******************\n");

    return check;
}

static void uneo_read_task(void *arg){
    UBaseType_t lastUxHighWaterMarkI2C; 
    lastUxHighWaterMarkI2C = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGI(UNEO_TAG, "uneo_read %d", uxHighWaterMarkI2C);
    uint8_t ret1= 0, ret2= 0, ret3 = 0;
    uint32_t err;
    uint8_t uneo_err;
    uint32_t task_delay = (uint32_t)arg;
    uint8_t *data = (uint8_t *)malloc(DATA_LENGTH * 3);

    while(1) {

        uneo_err = 0x00;
        ret1 = uneo_read(I2C_MASTER_NUM, UNEO_PANEL_1_ADDR, data);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        ret2 = uneo_read(I2C_MASTER_NUM, UNEO_PANEL_2_ADDR, data + DATA_LENGTH);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        ret3 = uneo_read(I2C_MASTER_NUM, UNEO_PANEL_3_ADDR, data + DATA_LENGTH * 2);
        vTaskDelay(20 / portTICK_PERIOD_MS);

        if(ret1 == ESP_ERR_TIMEOUT || ret2 == ESP_ERR_TIMEOUT || ret3 == ESP_ERR_TIMEOUT){
            uneo_err |= UNEO_ERROR_TIMEOUT;
        }else if((ret1 == ESP_OK || ret1 == 7) &&
                 (ret2 == ESP_OK || ret2 == 7) &&
                 (ret3 == ESP_OK || ret3 == 7)){
            
            bool check = uneo_data_check(UNEO_PANEL_1_ADDR, data) &
                        uneo_data_check(UNEO_PANEL_2_ADDR, data + DATA_LENGTH) &
                        uneo_data_check(UNEO_PANEL_3_ADDR, data + DATA_LENGTH * 2);
            
            if(check){
                size_t xBytesSent = xMessageBufferSend(uneo_data_message, data, DATA_LENGTH * 3, portMAX_DELAY);
                if(xBytesSent == DATA_LENGTH * 3){
                    // printf("*******************\n");
                    // for(int loop = 0; loop < DATA_LENGTH * 3; loop++)
                    //     printf("%d ", data[loop]);
                    // printf("*******************\n");
                }else{
                    ESP_LOGE(UNEO_TAG, "messageSend fail %d", xBytesSent);
                }
            }else{
                ESP_LOGE(UNEO_TAG, "Prefix and postfix check failed!");
                uneo_err |= UNEO_ERROR_PREFIX_FAIL;
            }
        }else {
            ESP_LOGE(UNEO_TAG, "I2C Timeout %04x, %04x, %04x, %04x", ESP_ERR_TIMEOUT, ret1, ret2, ret3);
            // ESP_LOGW(TAG, "%s: No ack, sensor not connected...skip...", esp_err_to_name(ret1));
            uneo_err |= UNEO_ERROR_NO_ACK;
        }

        if(uneo_err){
            // err = xMessageBufferSend(uneo_error_message, &uneo_err, sizeof(uneo_err), portMAX_DELAY);
        }         
    }
}
