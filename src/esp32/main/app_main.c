/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "app.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "ecu-pins.h"
#include "hal/ledc_types.h"

TaskHandle_t spi_task    = NULL;
TaskHandle_t uart_task   = NULL;
TaskHandle_t can_task    = NULL;
TaskHandle_t pwm_task    = NULL;
TaskHandle_t dac_task    = NULL;
TimerHandle_t adc_timer  = NULL;
TimerHandle_t hall_timer = NULL;
TaskHandle_t din_task    = NULL;
TaskHandle_t dout_task   = NULL;

static ESP32_In_Msg_t esp_in_status   = {};
static ESP32_Out_Msg_t esp_out_status = {.pwm[0].frequency       = 5000,
                                         .pwm[0].duty            = 0,
                                         .pwm[0].duty_resolution = LEDC_TIMER_1_BIT,
                                         .pwm[1].frequency       = 5000,
                                         .pwm[1].duty            = 0,
                                         .pwm[1].duty_resolution = LEDC_TIMER_1_BIT};

static ledc_timer_config_t pwm_0_timer = {.speed_mode      = LEDC_HIGH_SPEED_MODE,
                                          .timer_num       = LEDC_TIMER_0,
                                          .freq_hz         = 5000,
                                          .duty_resolution = LEDC_TIMER_1_BIT,
                                          .clk_cfg         = LEDC_AUTO_CLK};

static ledc_channel_config_t pwm_0_channel = {.speed_mode = LEDC_HIGH_SPEED_MODE,
                                              .channel    = LEDC_CHANNEL_0,
                                              .timer_sel  = LEDC_TIMER_0,
                                              .intr_type  = LEDC_INTR_DISABLE,
                                              .gpio_num   = ECU_PWM_1};

static ledc_timer_config_t pwm_1_timer = {.speed_mode      = LEDC_HIGH_SPEED_MODE,
                                          .timer_num       = LEDC_TIMER_1,
                                          .freq_hz         = 5000,
                                          .duty_resolution = LEDC_TIMER_1_BIT,
                                          .clk_cfg         = LEDC_AUTO_CLK};

static ledc_channel_config_t pwm_1_channel = {.speed_mode = LEDC_HIGH_SPEED_MODE,
                                              .channel    = LEDC_CHANNEL_1,
                                              .timer_sel  = LEDC_TIMER_1,
                                              .intr_type  = LEDC_INTR_DISABLE,
                                              .gpio_num   = ECU_PWM_2};

void run_spi(void *parameters)
{
#define print_status 0
    (void)parameters;

    // Initialise CRC calculation
    init_crc();

    static DMA_ATTR ECU_Msg_t tx_msg = {.name = "spi_ack", .data = {0}, .command = ACK_CMD};
    static DMA_ATTR ECU_Msg_t rx_msg = {0};

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
            ecu_send_rcv_status(&esp_in_status, &esp_out_status);
            vTaskResume(dout_task);
            vTaskResume(pwm_task);
            vTaskResume(dac_task);
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

    while (1)
    {
        uint8_t led_channels[2]                = {LEDC_CHANNEL_0, LEDC_CHANNEL_1};
        uint8_t led_timers[2]                  = {LEDC_TIMER_0, LEDC_TIMER_1};
        ledc_timer_config_t *pwm_timers[2]     = {&pwm_0_timer, &pwm_1_timer};
        ledc_channel_config_t *pwm_channels[2] = {&pwm_0_channel, &pwm_1_channel};

        for (int i = 0; i < 2; i++)
        {
            int pwm_enable   = esp_out_status.pwm[i].duty > 0;
            int pwm_reenable = pwm_enable && pwm_channels[i]->duty == 0;
            int pwm_update_resolution =
                pwm_enable && esp_out_status.pwm[i].duty_resolution != pwm_timers[i]->duty_resolution;
            int pwm_update_frequency = pwm_enable && esp_out_status.pwm[i].frequency != pwm_timers[i]->freq_hz;
            int pwm_update_duty      = pwm_enable && esp_out_status.pwm[i].duty != pwm_channels[i]->duty;

            pwm_timers[i]->freq_hz         = esp_out_status.pwm[i].frequency;
            pwm_timers[i]->duty_resolution = esp_out_status.pwm[i].duty_resolution;
            pwm_channels[i]->duty          = esp_out_status.pwm[i].duty;

            if (!pwm_enable && pwm_channels[i]->duty != 0)
            {
                ESP_ERROR_CHECK(ledc_stop(LEDC_HIGH_SPEED_MODE, led_channels[i], 0));
                ESP_ERROR_CHECK(ledc_timer_pause(LEDC_HIGH_SPEED_MODE, led_timers[i]));
            } else if (pwm_reenable || pwm_update_resolution || pwm_update_frequency)
            {
                ESP_ERROR_CHECK(ledc_stop(LEDC_HIGH_SPEED_MODE, led_channels[i], 0));
                ESP_ERROR_CHECK(ledc_timer_pause(LEDC_HIGH_SPEED_MODE, led_timers[i]));
                ESP_ERROR_CHECK(ledc_timer_config(pwm_timers[i]));
                ESP_ERROR_CHECK(ledc_channel_config(pwm_channels[i]));
                ESP_ERROR_CHECK(ledc_timer_resume(LEDC_HIGH_SPEED_MODE, led_timers[i]));
            } else if (pwm_update_duty)
            {
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, led_channels[i], pwm_0_channel.duty));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, led_channels[i]));
            }
        }

        vTaskSuspend(NULL);
    }
}

void run_dac(void *parameters)
{
    (void)parameters;
    while (1)
    {
        vTaskSuspend(NULL);
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
        uint32_t voltage     = esp_adc_cal_raw_to_voltage(adc_readings[j], &calib);
        esp_in_status.adc[j] = voltage;
    }
}

void run_hall_effect(void *parameters)
{
    (void)parameters;
    esp_in_status.hall_effect = hall_sensor_read();
}

void run_din(void *parameters)
{
    (void)parameters;
    while (1)
    {
        esp_in_status.din[0] = gpio_get_level(ECU_DIN_1);
        esp_in_status.din[1] = gpio_get_level(ECU_DIN_2);
        esp_in_status.din[2] = gpio_get_level(ECU_DIN_3);
        vTaskSuspend(NULL);
    }
}

void din_cb(void *params)
{
    (void)params;
    xTaskResumeFromISR(din_task);
}

void run_dout(void *parameters)
{
    (void)parameters;
    while (1)
    {
        gpio_set_level(ECU_DOUT_1, esp_out_status.dout[0]);
        vTaskSuspend(NULL);
    }
}

// Main application
void app_main(void)
{
    ecu_pins_init();

    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_1, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_2, &din_cb, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(ECU_DIN_3, &din_cb, NULL));

    xTaskCreate(run_spi, "run_spi", 4096, NULL, 5, &spi_task);

    xTaskCreate(run_uart, "run_uart", 4096, NULL, 2, &uart_task);

    xTaskCreate(run_can, "run_can", 4096, NULL, 5, &can_task);

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

    xTaskCreate(run_din, "run_din", 4096, NULL, 2, &din_task);

    xTaskCreate(run_dout, "run_dout", 4096, NULL, 2, &dout_task);

    ecu_log("RUNNING");

    vTaskSuspend(NULL);
}
