#ifndef APP_H
#define APP_H

#include "ESP32Msg.h"
#include "Message.h"
#include "driver/spi_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/timers.h"

#include "ecu-pins.h"
#include "ecu-shared.h"

#include <stdio.h>
#include <string.h>

void run_spi(void *pvParameters);
void run_uart(void *pvParameters);
void run_can(void *pvParameters);
void run_pwm(void *pvParameters);
void run_dac(void *pvParameters);
void run_adc(void *pvParameters);
void run_hall_effect(void *pvParameters);
void run_din(void *pvParameters);
void run_dout(void *pvParameters);

void log_esp_state(void *pvParameters);

void ecu_send_status(ESP32Msg_t *esp_status);

#endif
