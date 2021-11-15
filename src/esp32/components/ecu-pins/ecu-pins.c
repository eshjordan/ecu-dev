#include "ecu-pins.h"
#include "esp_err.h"
#include "soc/adc_channel.h"

static volatile int ecu_pins_initialized                        = 0;
static const volatile esp_adc_cal_characteristics_t ADC_1_CHARS = {};
static const volatile esp_adc_cal_characteristics_t ADC_2_CHARS = {};

static int init_spi(void)
{
    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num   = ECU_SPI_MOSI,
        .miso_io_num   = ECU_SPI_MISO,
        .sclk_io_num   = ECU_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {.mode          = 1,
                                           .spics_io_num  = ECU_SPI_CS,
                                           .queue_size    = 3,
                                           .flags         = 0,
                                           .post_setup_cb = NULL,
                                           .post_trans_cb = NULL};

    // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_SPI_MOSI, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_SPI_SCLK, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_SPI_CS, GPIO_PULLUP_ONLY));

    // Initialize SPI slave interface
    ESP_ERROR_CHECK(spi_slave_initialize(ECU_SPI_RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH1));

    return ESP_OK;
}

static int init_uart(void)
{
    const uart_port_t uart_num = UART_NUM_0;
    uart_config_t uart_config  = {
         .baud_rate = 115200,
         .data_bits = UART_DATA_8_BITS,
         .parity    = UART_PARITY_DISABLE,
         .stop_bits = UART_STOP_BITS_1,
         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(uart_num,           /* UART controller number. */
                                 ECU_UART_TX,        /* UART TX pin. */
                                 ECU_UART_RX,        /* UART RX pin. */
                                 UART_PIN_NO_CHANGE, /* UART RTS pin. */
                                 UART_PIN_NO_CHANGE  /* UART CTS pin. */
                                 ));

    return ESP_OK;
}

static int init_can(void)
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

static int init_pwm(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t pwm_1_timer = {.speed_mode      = LEDC_LOW_SPEED_MODE,
                                       .timer_num       = LEDC_TIMER_0,
                                       .duty_resolution = LEDC_TIMER_13_BIT,
                                       .freq_hz         = 5000, // Set output frequency at 5 kHz
                                       .clk_cfg         = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&pwm_1_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t pwm_1_channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                           .channel    = LEDC_CHANNEL_0,
                                           .timer_sel  = LEDC_TIMER_0,
                                           .intr_type  = LEDC_INTR_DISABLE,
                                           .gpio_num   = ECU_PWM_1,
                                           .duty       = 0, // Set duty to 0%
                                           .hpoint     = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&pwm_1_channel));

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t pwm_2_timer = {.speed_mode      = LEDC_LOW_SPEED_MODE,
                                       .timer_num       = LEDC_TIMER_0,
                                       .duty_resolution = LEDC_TIMER_13_BIT,
                                       .freq_hz         = 5000, // Set output frequency at 5 kHz
                                       .clk_cfg         = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&pwm_2_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t pwm_2_channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                           .channel    = LEDC_CHANNEL_0,
                                           .timer_sel  = LEDC_TIMER_0,
                                           .intr_type  = LEDC_INTR_DISABLE,
                                           .gpio_num   = ECU_PWM_2,
                                           .duty       = 0, // Set duty to 0%
                                           .hpoint     = 0};

    ESP_ERROR_CHECK(ledc_channel_config(&pwm_2_channel));

    return ESP_OK;
}

static int init_dac(void)
{
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_2));
    return ESP_OK;
}

static int init_adc(void)
{
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_12Bit));

    // ADC_1 pins
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_1), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_2), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_3), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_4), ECU_ADC_ATTENUATION));
#if !ECU_ENABLE_HALL_EFFECT
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_8), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_9), ECU_ADC_ATTENUATION));
#endif

    ESP_ERROR_CHECK(esp_adc_cal_characterize(ADC_UNIT_1, ECU_ADC_ATTENUATION, ADC_WIDTH_12Bit, 1093,
                                             (esp_adc_cal_characteristics_t *)&ADC_1_CHARS));

    // ADC_2 pins
    ESP_ERROR_CHECK(adc2_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_5), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc2_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_6), ECU_ADC_ATTENUATION));
    ESP_ERROR_CHECK(adc2_config_channel_atten(ECU_GET_ADC_CHANNEL(ECU_ADC_7), ECU_ADC_ATTENUATION));

    ESP_ERROR_CHECK(esp_adc_cal_characterize(ADC_UNIT_2, ECU_ADC_ATTENUATION, ADC_WIDTH_12Bit, 1093,
                                             (esp_adc_cal_characteristics_t *)&ADC_2_CHARS));

    return ESP_OK;
}

static int init_hall_effect(void)
{
#if !ECU_ENABLE_HALL_EFFECT
    return ESP_OK;
#endif
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_12Bit));
    return ESP_OK;
}

static int init_din(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = ECU_DIN_MASK,          /* GPIO pin: set with bit mask, each bit maps to a GPIO */
        .mode         = GPIO_MODE_INPUT,       /* GPIO mode: set input/output mode                     */
        .pull_up_en   = GPIO_PULLUP_ENABLE,    /* GPIO pull-up                                         */
        .pull_down_en = GPIO_PULLDOWN_DISABLE, /* GPIO pull-down                                       */
        .intr_type    = GPIO_INTR_ANYEDGE,     /* GPIO interrupt type                                  */
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_SHARED));

    return ESP_OK;
}

static int init_dout(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = ECU_DOUT_MASK,         /* GPIO pin: set with bit mask, each bit maps to a GPIO */
        .mode         = GPIO_MODE_OUTPUT,      /* GPIO mode: set input/output mode                     */
        .pull_up_en   = GPIO_PULLUP_DISABLE,   /* GPIO pull-up                                         */
        .pull_down_en = GPIO_PULLDOWN_DISABLE, /* GPIO pull-down                                       */
        .intr_type    = GPIO_INTR_DISABLE,     /* GPIO interrupt type                                  */
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    return ESP_OK;
}

esp_adc_cal_characteristics_t ecu_get_adc_calib(gpio_num_t gpio)
{
    switch (gpio)
    {
    case ADC1_CHANNEL_0_GPIO_NUM:
    case ADC1_CHANNEL_1_GPIO_NUM:
    case ADC1_CHANNEL_2_GPIO_NUM:
    case ADC1_CHANNEL_3_GPIO_NUM:
    case ADC1_CHANNEL_4_GPIO_NUM:
    case ADC1_CHANNEL_5_GPIO_NUM:
    case ADC1_CHANNEL_6_GPIO_NUM:
    case ADC1_CHANNEL_7_GPIO_NUM: return ADC_1_CHARS;

    case ADC2_CHANNEL_0_GPIO_NUM:
    case ADC2_CHANNEL_1_GPIO_NUM:
    case ADC2_CHANNEL_2_GPIO_NUM:
    case ADC2_CHANNEL_3_GPIO_NUM:
    case ADC2_CHANNEL_4_GPIO_NUM:
    case ADC2_CHANNEL_5_GPIO_NUM:
    case ADC2_CHANNEL_6_GPIO_NUM:
    case ADC2_CHANNEL_7_GPIO_NUM:
    case ADC2_CHANNEL_8_GPIO_NUM:
    case ADC2_CHANNEL_9_GPIO_NUM: return ADC_2_CHARS;

    default:
        ecu_error("ADC calibration not found");
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        esp_adc_cal_characteristics_t ret = {0};
        return ret;
    }
}

/* Initialisation function. */
int ecu_pins_init(void)
{
    if (ecu_pins_initialized)
    {
        ecu_warn("Tried to reinitialise ECU pins");
        return ESP_ERR_INVALID_STATE;
    }

    ecu_pins_initialized = 1;

    ecu_log("Initialising ECU pins");

    /* Initialise SPI. */
    init_spi();
    ecu_log("SPI initialised");

    /* Initialise UART. */
    init_uart();
    ecu_log("UART initialised");

    /* Initialise CAN. */
    init_can();
    ecu_log("CAN initialised");

    /* Initialise PWM. */
    init_pwm();
    ecu_log("PWM initialised");

    /* Initialise DAC. */
    init_dac();
    ecu_log("DAC initialised");

    /* Initialise ADC. */
    init_adc();
    ecu_log("ADC initialised");

    /* Initialise Hall Effect Sensor. */
    init_hall_effect();
    ecu_log("Hall Effect Sensor initialised");

    /* Initialise DIN. */
    init_din();
    ecu_log("DIN initialised");

    /* Initialise DOUT. */
    init_dout();
    ecu_log("DOUT initialised");

    ecu_log("ECU pins initialised successfully!");

    ecu_pins_initialized = 1;

    return ESP_OK;
}
