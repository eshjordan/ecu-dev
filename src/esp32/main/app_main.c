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
#include "CANSPI.h"

static SemaphoreHandle_t xSemaphore = NULL;
static StaticSemaphore_t xMutexBuffer;

static TaskHandle_t spi_task    = NULL;
static TaskHandle_t uart_task   = NULL;
static TaskHandle_t can_task    = NULL;
static TaskHandle_t pwm_task    = NULL;
static TaskHandle_t dac_task    = NULL;
static TimerHandle_t adc_timer  = NULL;
static TimerHandle_t hall_timer = NULL;
static TaskHandle_t din_task    = NULL;
static TaskHandle_t dout_task   = NULL;

static DMA_ATTR WORD_ALIGNED_ATTR ESP32_In_ADC_t adc_data[9]    = {0};
static DMA_ATTR WORD_ALIGNED_ATTR ESP32_In_Hall_t hall_data[1]  = {0};
static DMA_ATTR WORD_ALIGNED_ATTR ESP32_In_DIN_t din_data[3]    = {0};
static DMA_ATTR WORD_ALIGNED_ATTR ESP32_Out_DAC_t dac_data[2]   = {0};
static DMA_ATTR WORD_ALIGNED_ATTR ESP32_Out_PWM_t pwm_data[2]   = {0};
static DMA_ATTR WORD_ALIGNED_ATTR ESP32_Out_DOUT_t dout_data[1] = {0};

void spi_send_rcv_wait(void *tx_buf, void *rx_buf, size_t len)
{
    // clang-format off
    spi_slave_transaction_t trans_desc = {
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
        .length = len * 8
    };
    // clang-format on

    // Wait for the master to send a query, acknowledge we're here
    ESP_ERROR_CHECK(spi_slave_transmit(ECU_SPI_RCV_HOST, &trans_desc, portMAX_DELAY));
}

void spi_send_ack_wait()
{
    WORD_ALIGNED_ATTR ESP32_Request_t rx_buf  = {0};
    WORD_ALIGNED_ATTR ESP32_Request_t ack_msg = {0};
    ack_msg.seed                              = esp_random();
    ack_msg.type                              = ESP32_ACK;
    ack_msg.checksum                          = calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum));

    // clang-format off
    spi_slave_transaction_t trans_desc = {
        .tx_buffer = &ack_msg,
        .rx_buffer = &rx_buf,
        .length = sizeof(ESP32_Request_t) * 8
    };
    // clang-format on

    ESP_ERROR_CHECK(spi_slave_transmit(ECU_SPI_RCV_HOST, &trans_desc, portMAX_DELAY));
}

void spi_send_nack_wait()
{
    WORD_ALIGNED_ATTR ESP32_Request_t rx_buf   = {0};
    WORD_ALIGNED_ATTR ESP32_Request_t nack_msg = {0};
    nack_msg.seed                              = esp_random();
    nack_msg.type                              = ESP32_NACK;
    nack_msg.checksum                          = calc_crc(&nack_msg, offsetof(ESP32_Request_t, checksum));

    // clang-format off
    spi_slave_transaction_t trans_desc = {
        .tx_buffer = &nack_msg,
        .rx_buffer = &rx_buf,
        .length = sizeof(ESP32_Request_t) * 8
    };
    // clang-format on

    ESP_ERROR_CHECK(spi_slave_transmit(ECU_SPI_RCV_HOST, &trans_desc, portMAX_DELAY));
}

void run_spi(void *parameters)
{
    (void)parameters;

    // Initialise CRC calculation
    init_crc();

    while (1)
    {
        WORD_ALIGNED_ATTR uint8_t empty_buf[512] = {0};
        WORD_ALIGNED_ATTR ESP32_Request_t rx_msg = {0};
        spi_send_rcv_wait(empty_buf, &rx_msg, sizeof(ESP32_Request_t));

        if (rx_msg.checksum != calc_crc(&rx_msg, offsetof(ESP32_Request_t, checksum)))
        {
            ecu_warn("SPI - Received invalid message - bad CRC");
            continue;
        }

        spi_send_ack_wait();

        switch (rx_msg.type)
        {
        case ESP32_IN_ADC: {
            xSemaphoreTake(xSemaphore, portMAX_DELAY);

            adc_data[rx_msg.channel].seed     = esp_random();
            adc_data[rx_msg.channel].checksum = calc_crc(&adc_data[rx_msg.channel], offsetof(ESP32_In_ADC_t, checksum));
            spi_send_rcv_wait(&adc_data[rx_msg.channel], empty_buf, sizeof(ESP32_In_ADC_t));

            xSemaphoreGive(xSemaphore);
            break;
        }
        case ESP32_IN_HALL: {
            xSemaphoreTake(xSemaphore, portMAX_DELAY);

            hall_data[rx_msg.channel].seed = esp_random();
            hall_data[rx_msg.channel].checksum =
                calc_crc(&hall_data[rx_msg.channel], offsetof(ESP32_In_Hall_t, checksum));
            spi_send_rcv_wait(&hall_data, empty_buf, sizeof(ESP32_In_Hall_t));

            xSemaphoreGive(xSemaphore);
            break;
        }
        case ESP32_IN_DIN: {
            xSemaphoreTake(xSemaphore, portMAX_DELAY);

            din_data[rx_msg.channel].seed = esp_random();
            din_data[rx_msg.channel].checksum =
                calc_crc(&(din_data[rx_msg.channel]), offsetof(ESP32_In_DIN_t, checksum));
            spi_send_rcv_wait(&(din_data[rx_msg.channel]), empty_buf, sizeof(ESP32_In_DIN_t));

            xSemaphoreGive(xSemaphore);
            break;
        }
        case ESP32_OUT_DAC: {
            spi_send_rcv_wait(empty_buf, &dac_data[rx_msg.channel], sizeof(ESP32_Out_DAC_t));
            xTaskNotifyGive(dac_task);
            break;
        }
        case ESP32_OUT_PWM: {
            spi_send_rcv_wait(empty_buf, &pwm_data[rx_msg.channel], sizeof(ESP32_Out_PWM_t));
            xTaskNotifyGive(pwm_task);
            break;
        }
        case ESP32_OUT_DOUT: {
            spi_send_rcv_wait(empty_buf, &dout_data[rx_msg.channel], sizeof(ESP32_Out_DOUT_t));
            xTaskNotifyGive(dout_task);
            break;
        }
        case ESP32_IN_CAN:
        case ESP32_OUT_CAN:
        case ESP32_UNKNOWN:
        default: {
            ecu_warn("SPI - Received invalid command: Unknown");
            break;
        }
        }
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
    // vTaskSuspend(NULL);

    uCAN_MSG msg = {
        .frame.idType = 0,
        .frame.id = 0x123,
        .frame.dlc = 1,
        .frame.data0 = 0x69
    };

    while (1) {

        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        uint8_t success = CANSPI_Transmit(&msg);

        if (success != 1) {
            ecu_warn("CAN - Failed to transmit message");
        }

        ecu_log("CAN - Transmit message OK!");

        xSemaphoreGive(xSemaphore);

        vTaskDelay(pdMS_TO_TICKS(1000)); // TODO: Implement CAN

        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        uCAN_MSG recv_msg = {0};
        success = CANSPI_Receive(&recv_msg);

        if (success != 1) {
            ecu_warn("CAN - Failed to receive message");
        }

        ecu_log("CAN - Receive message OK! - ID: %x", recv_msg.frame.id);

        xSemaphoreGive(xSemaphore);
    }

}

void run_pwm(void *parameters)
{
    (void)parameters;

    // clang-format off
    static uint8_t led_channels[2]           = {LEDC_CHANNEL_0, LEDC_CHANNEL_1};
    static uint8_t led_timers[2]             = {LEDC_TIMER_0, LEDC_TIMER_1};
    static ledc_timer_config_t pwm_timers[2] = {
        {
            .speed_mode      = LEDC_HIGH_SPEED_MODE,
            .timer_num       = LEDC_TIMER_0,
            .freq_hz         = 5000,
            .duty_resolution = LEDC_TIMER_1_BIT,
            .clk_cfg         = LEDC_AUTO_CLK,
        },
        {
            .speed_mode      = LEDC_HIGH_SPEED_MODE,
            .timer_num       = LEDC_TIMER_1,
            .freq_hz         = 5000,
            .duty_resolution = LEDC_TIMER_1_BIT,
            .clk_cfg         = LEDC_AUTO_CLK,
        },
    };

    static ledc_channel_config_t pwm_channels[2] = {
        {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel    = LEDC_CHANNEL_0,
            .timer_sel  = LEDC_TIMER_0,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = ECU_PWM_1
        },
        {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel    = LEDC_CHANNEL_1,
            .timer_sel  = LEDC_TIMER_1,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = ECU_PWM_2
        }
    };
    // clang-format on

    while (1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        uint8_t resolutions[2] = {pwm_data[0].duty_resolution, pwm_data[1].duty_resolution};
        uint32_t freqs[2]      = {pwm_data[0].frequency, pwm_data[1].frequency};
        uint16_t duties[2]     = {pwm_data[0].duty, pwm_data[1].duty};
        xSemaphoreGive(xSemaphore);

        for (int i = 0; i < 2; i++)
        {
            int pwm_enable            = duties[i] > 0;
            int pwm_reenable          = pwm_enable && pwm_channels[i].duty == 0;
            int pwm_update_resolution = pwm_enable && resolutions[i] != pwm_timers[i].duty_resolution;
            int pwm_update_frequency  = pwm_enable && freqs[i] != pwm_timers[i].freq_hz;
            int pwm_update_duty       = pwm_enable && duties[i] != pwm_channels[i].duty;

            pwm_timers[i].freq_hz         = freqs[i];
            pwm_timers[i].duty_resolution = resolutions[i];
            pwm_channels[i].duty          = duties[i];

            if (!pwm_enable && pwm_channels[i].duty != 0)
            {
                ESP_ERROR_CHECK(ledc_stop(LEDC_HIGH_SPEED_MODE, led_channels[i], 0));
                ESP_ERROR_CHECK(ledc_timer_pause(LEDC_HIGH_SPEED_MODE, led_timers[i]));
            } else if (pwm_reenable || pwm_update_resolution || pwm_update_frequency)
            {
                ESP_ERROR_CHECK(ledc_stop(LEDC_HIGH_SPEED_MODE, led_channels[i], 0));
                ESP_ERROR_CHECK(ledc_timer_pause(LEDC_HIGH_SPEED_MODE, led_timers[i]));
                ESP_ERROR_CHECK(ledc_timer_config(&pwm_timers[i]));
                ESP_ERROR_CHECK(ledc_channel_config(&pwm_channels[i]));
                ESP_ERROR_CHECK(ledc_timer_resume(LEDC_HIGH_SPEED_MODE, led_timers[i]));
            } else if (pwm_update_duty)
            {
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, led_channels[i], pwm_channels[i].duty));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, led_channels[i]));
            }
        }

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void run_dac(void *parameters)
{
    (void)parameters;
    const double vdd_a = 3.3;

    while (1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        uint8_t dac_1_value = (255 * dac_data[0].dac) / vdd_a;
        ESP_ERROR_CHECK(dac_output_voltage(DAC_CHANNEL_1, dac_1_value));

        uint8_t dac_2_value = (255 * dac_data[1].dac) / vdd_a;
        ESP_ERROR_CHECK(dac_output_voltage(DAC_CHANNEL_2, dac_2_value));

        xSemaphoreGive(xSemaphore);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
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
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_readings[j], &calib);

        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        adc_data[j].adc = voltage;
        xSemaphoreGive(xSemaphore);
    }
}

void run_hall_effect(void *parameters)
{
    (void)parameters;
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    hall_data[0].hall = hall_sensor_read();
    xSemaphoreGive(xSemaphore);
}

void run_din(void *parameters)
{
    (void)parameters;
    while (1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        din_data[0].din = gpio_get_level(ECU_DIN_1);
        din_data[1].din = gpio_get_level(ECU_DIN_2);
        din_data[2].din = gpio_get_level(ECU_DIN_3);
        xSemaphoreGive(xSemaphore);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void din_cb(void *params)
{
    (void)params;
    vTaskNotifyGiveFromISR(din_task, NULL);
}

void run_dout(void *parameters)
{
    (void)parameters;
    while (1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        gpio_set_level(ECU_DOUT_1, dout_data[0].dout);
        xSemaphoreGive(xSemaphore);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

// Main application
void app_main(void)
{
    xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    ecu_pins_init();

    // CANSPI_Initialize();

    // ecu_log("Finished CANSPI_Initialize()");

    vTaskDelay(pdMS_TO_TICKS(500));

    // TODO: DIN conflicts with CAN SPI
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_1, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_2, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_3, &din_cb, NULL));

    xTaskCreate(run_spi, "run_spi", 4096, NULL, 5, &spi_task);

    xTaskCreate(run_uart, "run_uart", 4096, NULL, 2, &uart_task);

    // xTaskCreate(run_can, "run_can", 4096, NULL, 5, &can_task);

    xTaskCreate(run_pwm, "run_pwm", 4096, NULL, 4, &pwm_task);

    xTaskCreate(run_dac, "run_dac", 4096, NULL, 4, &dac_task);

    adc_timer = xTimerCreate("run_adc", pdMS_TO_TICKS(1000), pdTRUE, 0, run_adc);

    if (adc_timer == NULL)
    {
        ecu_error("ADC TIMER FAILED - ADC READING DISABLED");
    } else
    {
        xTimerStart(adc_timer, 0);
    }

    hall_timer = xTimerCreate("run_hall_effect", pdMS_TO_TICKS(1000), pdTRUE, 0, run_hall_effect);

    if (hall_timer == NULL)
    {
        ecu_error("HALL TIMER FAILED - HALL READING DISABLED");
    } else
    {
        xTimerStart(hall_timer, 0);
    }

    // TODO: DIN conflicts with CAN SPI
    xTaskCreate(run_din, "run_din", 4096, NULL, 2, &din_task);

    xTaskCreate(run_dout, "run_dout", 4096, NULL, 2, &dout_task);

    ecu_log("RUNNING");

    vTaskSuspend(NULL);
}
