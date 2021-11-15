#ifndef APP_H
#define APP_H

#include "ECU_Msg.h"
#include "ESP32_In_Msg.h"
#include "driver/spi_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/timers.h"

#include "ecu-pins.h"
#include "ecu-shared.h"

#include <stdio.h>
#include <string.h>

void run_spi(void *parameters);
void run_uart(void *parameters);
void run_can(void *parameters);
void run_pwm(void *parameters);
void run_dac(void *parameters);
void run_adc(void *parameters);
void run_hall_effect(void *parameters);
void run_din(void *parameters);
void run_dout(void *parameters);

void log_esp_state(void *parameters);

void ecu_send_status(ESP32_In_Msg_t *esp_status);

#endif
