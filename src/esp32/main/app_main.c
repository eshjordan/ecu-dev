/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "app.h"

static ESP32_In_Msg_t esp_status = {};

void run_spi(void *parameters)
{
#define print_status 0
    (void)parameters;

    // Initialise CRC calculation
    init_crc();

    static DMA_ATTR WORD_ALIGNED_ATTR ECU_Msg_t tx_msg = {.name = "spi_ack", .data = {0}, .command = ACK_CMD};
    static DMA_ATTR WORD_ALIGNED_ATTR ECU_Msg_t rx_msg = {0};

    // clang-format off
    static spi_slave_transaction_t trans_desc = {
        .tx_buffer = &tx_msg,
        .rx_buffer = &rx_msg,
        .length = sizeof(ECU_Msg_t) * 8
    };
    // clang-format on

    static char msg[1024] = {0};

    while (1)
    {
        tx_msg.header = header_make(0, sizeof(ECU_Msg_t));
        ecu_msg_calc_checksum(&tx_msg);

        // Clear the rx buffer
        memset(&rx_msg, 0, sizeof(ECU_Msg_t));

        // Wait for the master to send a query, acknowledge we're here
        ESP_ERROR_CHECK(spi_slave_transmit(ECU_SPI_RCV_HOST, &trans_desc, portMAX_DELAY));

#if print_status
        msg_to_str(msg, &rx_msg);
        ecu_warn("Rx msg:\n%s\n", msg);
#endif

        ecu_err_t msg_status = ecu_msg_check(&rx_msg);
        if (msg_status < 0)
        {
            ecu_err_to_str(msg, msg_status);
            ecu_warn("SPI - Received invalid message - %s", msg);
            goto nd;
        }

        switch (rx_msg.command)
        {
        case STATUS_CMD: {
            ecu_send_status(&esp_status);
            break;
        }
        default: {
            ecu_warn("SPI - Received invalid command: %d", rx_msg.command);
            goto nd;
        }
        }

    nd:
        NULL;
    }
}

void run_uart(void *parameters)
{
    (void)parameters;
    vTaskSuspend(NULL);
}

void run_can(void *parameters)
{
    (void)parameters;
    vTaskSuspend(NULL);
}

void run_pwm(void *parameters)
{
    (void)parameters;
    vTaskSuspend(NULL);
}

void run_dac(void *parameters)
{
    (void)parameters;
    vTaskSuspend(NULL);
}

void run_adc(void *parameters)
{
    (void)parameters;
    static const uint8_t num_samples = 64;

    static const uint8_t num_adc_1 = 4;
    static const uint8_t num_adc_2 = 3;

    static const adc_channel_t adc_gpios[] = {ECU_GET_ADC_CHANNEL(ECU_ADC_1), ECU_GET_ADC_CHANNEL(ECU_ADC_2),
                                              ECU_GET_ADC_CHANNEL(ECU_ADC_3), ECU_GET_ADC_CHANNEL(ECU_ADC_4),
                                              ECU_GET_ADC_CHANNEL(ECU_ADC_5), ECU_GET_ADC_CHANNEL(ECU_ADC_6),
                                              ECU_GET_ADC_CHANNEL(ECU_ADC_7)};

    uint32_t adc_readings[7] = {};

    // Multisampling
    for (uint8_t i = 0; i < num_samples; i++)
    {
        for (uint8_t j = 0; j < num_adc_1 + num_adc_2; j++)
        {
            if (j < num_adc_1)
            {
                adc_readings[j] += adc1_get_raw((adc1_channel_t)adc_gpios[j]);
            } else
            {
                int raw = 0;
                ESP_ERROR_CHECK(adc2_get_raw((adc2_channel_t)adc_gpios[j], ADC_WIDTH_12Bit, &raw));
                adc_readings[j] += raw;
            }
        }
    }

    for (uint8_t j = 0; j < num_adc_1 + num_adc_2; j++)
    {
        adc_readings[j] /= num_samples;
        esp_adc_cal_characteristics_t calib = ecu_get_adc_calib(j < num_adc_1 ? ECU_ADC_1 : ECU_ADC_2);

        // Convert adc_reading to voltage in mV
        uint32_t voltage  = esp_adc_cal_raw_to_voltage(adc_readings[j], &calib);
        esp_status.adc[j] = voltage;
    }
}

void run_hall_effect(void *parameters)
{
    (void)parameters;
    vTaskSuspend(NULL);
}

void run_din(void *parameters)
{
    (void)parameters;
    while (1)
    {
        esp_status.din[0] = gpio_get_level(ECU_DIN_1);
        esp_status.din[1] = gpio_get_level(ECU_DIN_2);
        esp_status.din[2] = gpio_get_level(ECU_DIN_3);
        vTaskSuspend(NULL);
    }
}

void run_dout(void *parameters)
{
    (void)parameters;
    while (1)
    {
        // gpio_set_level(ECU_DOUT_1, esp_status.dout1);
        vTaskSuspend(NULL);
    }
}

void log_esp_state(void *parameters)
{
    (void)parameters;
    static char buf[1024] = {};

    esp32_in_msg_to_str(buf, &esp_status);
    ecu_log("%s", buf);
}

void din_cb(void *params)
{
    (void)params;
    xTaskResumeFromISR(run_din);
}

// Main application
void app_main(void)
{
    ecu_pins_init();

    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_1, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_2, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_3, &din_cb, NULL));

    TaskHandle_t spi_task   = NULL;
    TaskHandle_t uart_task  = NULL;
    TaskHandle_t can_task   = NULL;
    TaskHandle_t pwm_task   = NULL;
    TaskHandle_t dac_task   = NULL;
    TimerHandle_t adc_timer = NULL;
    TaskHandle_t hall_task  = NULL;
    TaskHandle_t din_task   = NULL;
    TaskHandle_t dout_task  = NULL;
    TimerHandle_t log_timer = NULL;

    xTaskCreate(run_spi, "run_spi", 4096, NULL, 5, &spi_task);

    xTaskCreate(run_uart, "run_uart", 4096, NULL, 2, &uart_task);

    xTaskCreate(run_can, "run_can", 4096, NULL, 5, &can_task);

    xTaskCreate(run_pwm, "run_pwm", 4096, NULL, 4, &pwm_task);

    xTaskCreate(run_dac, "run_dac", 4096, NULL, 4, &dac_task);

    adc_timer = xTimerCreate("run_adc", pdMS_TO_TICKS(1000), pdTRUE, 0, run_adc);

    if (adc_timer == NULL)
    {
        ecu_log("ADC TIMER FAILED - ADC READING DISABLED");
    } else
    {
        xTimerStart(adc_timer, 0);
    }

    xTaskCreate(run_hall_effect, "run_hall_effect", 4096, NULL, 2, &hall_task);

    xTaskCreate(run_din, "run_din", 4096, NULL, 2, &din_task);

    xTaskCreate(run_dout, "run_dout", 4096, NULL, 2, &dout_task);

    // log_timer = xTimerCreate("log_esp_state", pdMS_TO_TICKS(1000), pdTRUE, 0, log_esp_state);

    // if (log_timer == NULL)
    // {
    //     ecu_log("LOG TIMER FAILED - STATUS LOGGING DISABLED\n");
    // } else
    // {
    //     xTimerStart(log_timer, 0);
    // }

    ecu_log("RUNNING");

    vTaskSuspend(NULL);
}
