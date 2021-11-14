#ifndef ECU_PINS_H
#define ECU_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"
#include "driver/dac.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_slave.h"
#include "driver/twai.h"
#include "driver/uart.h"

// #include "hal/gpio_types.h"

#include "esp_adc_cal.h"

/* Useful macros. */
#define ecu_log(fmt_str, ...) printf("\033[36;1m[ECU_LOG  ] - \033[36m" fmt_str "\033[0m\n", ##__VA_ARGS__)
#define ecu_warn(fmt_str, ...) printf("\033[33;1m[ECU_WARN ] - \033[33m" fmt_str "\033[0m\n", ##__VA_ARGS__)
#define ecu_error(fmt_str, ...) printf("\033[31;1m[ECU_ERROR] - \033[31m" fmt_str "\033[0m\n", ##__VA_ARGS__)
#define ecu_debug(fmt_str, ...) printf("\033[1m[ECU_DEBUG] - \033[0m" fmt_str "\n", ##__VA_ARGS__)

/* Extra configuration. */
#define ECU_SPI_RCV_HOST HSPI_HOST
#define ECU_ADC_ATTENUATION ADC_ATTEN_DB_11 // ~ 3.55x 800mV (2.84V)
#define ECU_ENABLE_HALL_EFFECT (1)

/* Pin assignments. DO NOT USE LEADING ZEROS PLEASE!!
 * ESP32-DevKitC v4 Pinout:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/_images/esp32-devkitC-v4-pinout.jpg */

/* Serial Peripheral Interface (SPI) */
#define ECU_SPI_MISO GPIO_NUM_12
#define ECU_SPI_MOSI GPIO_NUM_13
#define ECU_SPI_SCLK GPIO_NUM_14
#define ECU_SPI_CS GPIO_NUM_15
#define ECU_SPI_MASK (GPIO_SEL_12 | GPIO_SEL_13 | GPIO_SEL_14 | GPIO_SEL_15)

/* Universal Asynchronous Receive + Transmit (UART).
 * Potential future changes:
 *  - Can use more than one UART controller on more pins
 *  - Use hardware flow control to improve reliability
 */
#define ECU_UART_TX GPIO_NUM_1
#define ECU_UART_RX GPIO_NUM_3
#define ECU_UART_MASK (GPIO_SEL_1 | GPIO_SEL_3)

/* Controller Area Network (CAN), AKA Two Wire Automotive Interface (TWAI). */
#define ECU_CAN_TX GPIO_NUM_21
#define ECU_CAN_RX GPIO_NUM_22
#define ECU_CAN_MASK (GPIO_SEL_21 | GPIO_SEL_22)

/* Pulse Width Modulation (PWM). */
#define ECU_PWM_1 GPIO_NUM_16
#define ECU_PWM_2 GPIO_NUM_17
#define ECU_PWM_MASK (GPIO_SEL_16 | GPIO_SEL_17)

/* Analogue to Digital Converter (ADC). */
#define ECU_ADC_1 GPIO_NUM_34
#define ECU_ADC_2 GPIO_NUM_35
#define ECU_ADC_3 GPIO_NUM_32
#define ECU_ADC_4 GPIO_NUM_33
#define ECU_ADC_5 GPIO_NUM_27
#define ECU_ADC_6 GPIO_NUM_4
#define ECU_ADC_7 GPIO_NUM_2

/* Internal Hall Effect sensor needs these pins. If you aren't going to use it, you can also use these as ADCs. */
#if ECU_ENABLE_HALL_EFFECT
#define ECU_HALL_1 GPIO_NUM_36
#define ECU_HALL_2 GPIO_NUM_39
#define ECU_HALL_MASK (GPIO_SEL_36 | GPIO_SEL_39)

#define ECU_ADC_MASK (GPIO_SEL_34 | GPIO_SEL_35 | GPIO_SEL_32 | GPIO_SEL_33 | GPIO_SEL_27 | GPIO_SEL_4 | GPIO_SEL_2)

#else
#define ECU_ADC_8 GPIO_NUM_36
#define ECU_ADC_9 GPIO_NUM_39
#define ECU_ADC_MASK                                                                                                   \
    (GPIO_SEL_34 | GPIO_SEL_35 | GPIO_SEL_32 | GPIO_SEL_33 | GPIO_SEL_27 | GPIO_SEL_4 | GPIO_SEL_2 | GPIO_SEL_36       \
     | GPIO_SEL_39)

#endif

#if ECU_ENABLE_HALL_EFFECT
#define ECU_GET_ADC_CHANNEL(gpio)                                                                                      \
    gpio == GPIO_NUM_34   ? ADC1_CHANNEL_6                                                                             \
    : gpio == GPIO_NUM_35 ? ADC1_CHANNEL_7                                                                             \
    : gpio == GPIO_NUM_32 ? ADC1_CHANNEL_4                                                                             \
    : gpio == GPIO_NUM_33 ? ADC1_CHANNEL_5                                                                             \
    : gpio == GPIO_NUM_27 ? ADC2_CHANNEL_7                                                                             \
    : gpio == GPIO_NUM_4  ? ADC2_CHANNEL_0                                                                             \
    : gpio == GPIO_NUM_2  ? ADC2_CHANNEL_2                                                                             \
                          : -1
#else
#define ECU_GET_ADC_CHANNEL(gpio)                                                                                      \
    gpio == GPIO_NUM_34   ? ADC1_CHANNEL_6                                                                             \
    : gpio == GPIO_NUM_35 ? ADC1_CHANNEL_7                                                                             \
    : gpio == GPIO_NUM_32 ? ADC1_CHANNEL_4                                                                             \
    : gpio == GPIO_NUM_33 ? ADC1_CHANNEL_5                                                                             \
    : gpio == GPIO_NUM_27 ? ADC2_CHANNEL_7                                                                             \
    : gpio == GPIO_NUM_4  ? ADC2_CHANNEL_0                                                                             \
    : gpio == GPIO_NUM_2  ? ADC2_CHANNEL_2                                                                             \
    : gpio == GPIO_NUM_36 ? ADC1_CHANNEL_0                                                                             \
    : gpio == GPIO_NUM_39 ? ADC1_CHANNEL_3                                                                             \
                          : -1
#endif

/* Digital to Analogue Converter (DAC). */
#define ECU_DAC_1 GPIO_NUM_25
#define ECU_DAC_2 GPIO_NUM_26
#define ECU_DAC_MASK (GPIO_SEL_25 | GPIO_SEL_26)

/* Digital Input (DIN). */
#define ECU_DIN_1 GPIO_NUM_5
#define ECU_DIN_2 GPIO_NUM_18
#define ECU_DIN_3 GPIO_NUM_19
#define ECU_DIN_MASK (GPIO_SEL_5 | GPIO_SEL_18 | GPIO_SEL_19)

/* Digital Output (DOUT). */
#define ECU_DOUT_1 GPIO_NUM_23
#define ECU_DOUT_MASK (GPIO_SEL_23)

/* Pin connected to the BOOT button on the ESP32-DevKitC board. Probably best not to use this, unless flashing. */
#define ECU_BOOT_PIN GPIO_NUM_0
#define ECU_BOOT_PIN_MASK (GPIO_SEL_0)

/* Don't use these yet, not sure about their suitability. */
#define ECU_RESERVED_1 GPIO_NUM_6
#define ECU_RESERVED_2 GPIO_NUM_7
#define ECU_RESERVED_3 GPIO_NUM_8
#define ECU_RESERVED_4 GPIO_NUM_9
#define ECU_RESERVED_5 GPIO_NUM_10
#define ECU_RESERVED_6 GPIO_NUM_11
#define ECU_RESERVED_MASK (GPIO_SEL_6 | GPIO_SEL_7 | GPIO_SEL_8 | GPIO_SEL_9 | GPIO_SEL_10 | GPIO_SEL_11)

esp_adc_cal_characteristics_t ecu_get_adc_calib(gpio_num_t gpio);

/* Initialisation function. */
int ecu_pins_init(void);

#ifdef __cplusplus
}
#endif

#endif
