/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "CRC.h"
#include "ESP32_Msg.h"
#include "app.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/spi_slave.h"
#include "ecu-pins.h"
#include "esp_attr.h"
#include "esp_random.h"
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"

static SemaphoreHandle_t xSemaphore = NULL;
static StaticSemaphore_t xMutexBuffer;

static TaskHandle_t can_task    = NULL;

static DMA_ATTR WORD_ALIGNED_ATTR ESP32_In_ADC_t adc_data[9]    = {0};

esp_err_t handle_can_state(void)
{
    esp_err_t err = ESP_FAIL;

    twai_status_info_t status;
    ESP_ERROR_CHECK(twai_get_status_info(&status));

    switch (status.state) {
    case TWAI_STATE_STOPPED:
    {
        // Start the CAN bus
        err = twai_start();
        break;
    }
    case TWAI_STATE_RUNNING:
    {
        err = ESP_OK;
        break;
    }
    case TWAI_STATE_BUS_OFF:
    {
        // Recover the CAN bus
        err = twai_initiate_recovery();
        if (err == ESP_OK) {
            ecu_log("CAN - Recovery successfully initiated");
        } else {
            ecu_warn("CAN - TWAI driver is not in the bus-off state, or is not installed");
        }
        break;
    }
    case TWAI_STATE_RECOVERING:
    default:
    {
        break;
    }
    }

    return err;
}

void run_can_tx(void *parameters)
{
    (void)parameters;

    //Configure message to transmit
    twai_message_t tx_message;
    tx_message.identifier = 0x123;
    tx_message.extd = 0;
    tx_message.data_length_code = 4;
    for (uint8_t i = 0; i < 4; i++) {
        tx_message.data[i] = i;
    }

    while (1) {

        esp_err_t can_state_ok = handle_can_state();
        if (can_state_ok != ESP_OK)
        {
            ecu_log("CAN - State not ok for TX");
            goto end_run_can_tx;
        }

        esp_err_t tx_result = twai_transmit(&tx_message, pdMS_TO_TICKS(1000));
        switch (tx_result) {
        case ESP_OK:
            ecu_log("CAN - Transmission successfully queued/initiated");
            break;
        case ESP_ERR_INVALID_ARG:
            ecu_warn("CAN - Arguments are invalid");
            goto end_run_can_tx;
        case ESP_ERR_TIMEOUT:
            ecu_warn("CAN - Timed out waiting for space on TX queue");
            if (ESP_OK == twai_clear_transmit_queue())
            {
                ecu_log("CAN - Transmission queue cleared");
            }
            else
            {
                ecu_warn("CAN - Failed to clear TX queue");
            }
            goto end_run_can_tx;
        case ESP_FAIL:
            ecu_warn("CAN - TX queue is disabled and another message is currently transmitting");
            goto end_run_can_tx;
        case ESP_ERR_INVALID_STATE:
            ecu_warn("CAN - TWAI driver is not in running state, or is not installed");
            goto end_run_can_tx;
        case ESP_ERR_NOT_SUPPORTED:
            ecu_warn("CAN - Listen Only Mode does not support transmissions");
            goto end_run_can_tx;
        default:
            ecu_warn("CAN - Unknown TX error");
            goto end_run_can_tx;
        }

end_run_can_tx:
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

void run_can_rx(void *parameters) {
    (void)parameters;

    while (1) {
        esp_err_t can_state_ok = handle_can_state();
        if (can_state_ok != ESP_OK)
        {
            ecu_log("CAN - State not ok for TX");
            goto end_run_can_rx;
        }

        twai_message_t rx_message;
        esp_err_t rx_result = twai_receive(&rx_message, pdMS_TO_TICKS(1000));
        switch (rx_result) {
        case ESP_OK:
            ecu_log("CAN - Message successfully received from RX queue");
            break;
        case ESP_ERR_TIMEOUT:
            ecu_warn("CAN - Timed out waiting for message");
            goto end_run_can_rx;
        case ESP_ERR_INVALID_ARG:
            ecu_warn("CAN - Arguments are invalid");
            goto end_run_can_rx;
        case ESP_ERR_INVALID_STATE:
            ecu_warn("CAN - TWAI driver is not installed");
            goto end_run_can_rx;
        default:
            ecu_warn("CAN - Unknown RX error");
            goto end_run_can_rx;
        }

        //Process received message
        if (rx_message.extd) {
            printf("Message is in Extended Format\n");
        } else {
            printf("Message is in Standard Format\n");
        }
        printf("ID is %d\n", rx_message.identifier);
        if (!(rx_message.rtr)) {
            for (int i = 0; i < rx_message.data_length_code; i++) {
                printf("Data byte %d = %d\n", i, rx_message.data[i]);
            }
        }

        ecu_log("CAN - Receive message OK! - ID: %x", rx_message.identifier);

end_run_can_rx:
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static int init_can_test(void)
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(ECU_CAN_TX, ECU_CAN_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config  = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config  = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

    // Start TWAI driver
    ESP_ERROR_CHECK(twai_start());

    return ESP_OK;
}

// Main application
void app_main(void)
{
    xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);

    init_can_test();

    vTaskDelay(pdMS_TO_TICKS(500));

    xTaskCreate(run_can_tx, "run_can", 4096, NULL, 5, &can_task);

    ecu_log("RUNNING");

    vTaskSuspend(NULL);
}
