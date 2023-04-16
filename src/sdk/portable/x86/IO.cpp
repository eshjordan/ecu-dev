#include "IO.hpp"
#include "CRC.h"
#include "ESP32_Msg.h"
#include "Header.h"
#include "System.hpp"
#include <cstdio>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SPI_WAIT_TIME pdMS_TO_TICKS(1)

static TaskHandle_t in_update_handle = {};
static TaskHandle_t out_update_handle = {};

static StackType_t in_update_stack[4] = {};
static StackType_t out_update_stack[4] = {};

static StaticTask_t in_update_tsk_buf = {};
static StaticTask_t out_update_tsk_buf = {};

static bool esp32_spi_inited = false;
static bool can_inited = false;


SemaphoreHandle_t esp32DataMutexHandle;

static ALIGN ESP32_In_ADC_t adc_data[9] = { 0 };
static ALIGN ESP32_In_Hall_t hall_data[1] = { 0 };
static ALIGN ESP32_In_DIN_t din_data[3] = { 0 };
static ALIGN ESP32_Out_DAC_t dac_data[2] = { 0 };
static ALIGN ESP32_Out_PWM_t pwm_data[2] = { 0 };
static ALIGN ESP32_Out_DOUT_t dout_data[1] = { 0 };

static int init_esp32_spi(void)
{
	printf("SPI init success\n");

	esp32_spi_inited = true;

	return 1;
}

void request_peripheral(uint8_t type, uint8_t channel)
{
	ALIGN ESP32_Request_t rx_buf = { 0 };
	ALIGN ESP32_Request_t tx_msg = { 0 };
	tx_msg.seed = rand();
	tx_msg.type = type;
	tx_msg.channel = channel;
	tx_msg.checksum =
		calc_crc(&tx_msg, offsetof(ESP32_Request_t, checksum));

	System::IO::spi_transfer(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
				 &tx_msg, &rx_buf);
}

bool receive_ack(void)
{
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		return false;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		return false;
	}

	return true;
}

void update_adc(IOADCChannel_en channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_IN_ADC, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	bool ack = receive_ack();
	vTaskDelay(SPI_WAIT_TIME);

	if (!ack) {
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	// Receive data
	ESP32_In_ADC_t adc_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_In_ADC_t),
			     &adc_msg);

	if (adc_msg.checksum !=
	    calc_crc(&adc_msg, offsetof(ESP32_In_ADC_t, checksum))) {
		printf("SPI - Received invalid adc msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	adc_data[channel].adc = adc_msg.adc;

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

void update_hall(IOHallChannel channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_IN_HALL, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	ESP32_In_Hall_t hall_msg = { 0 };

	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_In_Hall_t),
			     &hall_msg);

	if (hall_msg.checksum !=
	    calc_crc(&hall_msg, offsetof(ESP32_In_Hall_t, checksum))) {
		printf("SPI - Received invalid hall msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	hall_data[channel].hall = hall_msg.hall;

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

void update_din(IODINChannel_en channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_IN_DIN, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	ESP32_In_DIN_t din_msg = { 0 };

	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_In_DIN_t),
			     &din_msg);

	if (din_msg.checksum !=
	    calc_crc(&din_msg, offsetof(ESP32_In_DIN_t, checksum))) {
		printf("SPI - Received invalid hall msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	din_data[channel].din = din_msg.din;

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

void update_dac(IODACChannel_en channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_OUT_DAC, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	dac_data[channel].seed = rand();
	dac_data[channel].checksum = calc_crc(
		&dac_data[channel], offsetof(ESP32_Out_DAC_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_DAC_t),
			      &dac_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

void update_pwm(IOPWMChannel_en channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_OUT_PWM, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	pwm_data[channel].seed = rand();
	pwm_data[channel].checksum = calc_crc(
		&pwm_data[channel], offsetof(ESP32_Out_PWM_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_PWM_t),
			      &pwm_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

void update_dout(IODOUTChannel channel)
{
	xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_OUT_DOUT, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	ALIGN ESP32_Request_t ack_msg = { 0 };
	System::IO::spi_read(IO_SPI_CHANNEL_01, sizeof(ESP32_Request_t),
			     &ack_msg);

	if (ack_msg.checksum !=
	    calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum))) {
		printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		xSemaphoreGive(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	dout_data[channel].seed = rand();
	dout_data[channel].checksum = calc_crc(
		&dout_data[channel], offsetof(ESP32_Out_DOUT_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_DOUT_t),
			      &dout_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	xSemaphoreGive(esp32DataMutexHandle);
}

extern "C" void esp_in_update(TimerHandle_t handle)
{
	//	while(1)
	//	{
	// Inputs
	for (int i = IO_ADC_CHANNEL_01; i < ARRAY_SIZE(adc_data); i++) {
		update_adc((IOADCChannel_en)i);
	}

	for (int i = IO_HALL_CHANNEL_01; i < ARRAY_SIZE(hall_data); i++) {
		update_hall((IOHallChannel)i);
	}

	for (int i = IO_DIN_CHANNEL_01; i < ARRAY_SIZE(din_data); i++) {
		update_din((IODINChannel_en)i);
	}

	//		vTaskDelay(pdMS_TO_TICKS(16));
	//	}
}

extern "C" void esp_out_update(TimerHandle_t handle)
{
	//	while(1)
	//	{
	// Outputs
	for (int i = IO_DAC_CHANNEL_01; i < ARRAY_SIZE(dac_data); i++) {
		update_dac((IODACChannel_en)i);
	}

	for (int i = IO_PWM_CHANNEL_01; i < ARRAY_SIZE(pwm_data); i++) {
		update_pwm((IOPWMChannel_en)i);
	}

	for (int i = IO_DOUT_CHANNEL_01; i < ARRAY_SIZE(dout_data); i++) {
		update_dout((IODOUTChannel)i);
	}

	//		vTaskDelay(pdMS_TO_TICKS(16));
	//	}
}

namespace System
{
namespace IO
{

int port_init_io(void)
{
	//    xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);

	if (!esp32_spi_inited) {
		if (init_esp32_spi() < 0) {
			return -1;
		}
	}

    esp32DataMutexHandle = xSemaphoreCreateMutex();

	// if (!can_inited) {
	//     uint8_t status = 0;
	//     status = CANSPI_Initialize();
	//     can_inited = 1 == status;
	// }

	//    in_update_handle = xTaskCreateStatic(
	//    		esp_in_update,
	//    		"in_update_tsk",
	//			4,
	//			NULL,
	//			4,
	//			in_update_stack,
	//			&in_update_tsk_buf
	//    );
	//
	//    out_update_handle = xTaskCreateStatic(
	//    		esp_out_update,
	//    		"out_update_tsk",
	//			4,
	//			NULL,
	//			4,
	//			out_update_stack,
	//			&out_update_tsk_buf
	//    );

	return 1;
}

uint32_t read_analogue_input(IOADCChannel_en channel)
{
	return adc_data[channel].adc;
}

uint32_t read_hall_input(IOHallChannel channel)
{
	return hall_data[channel].hall;
}

uint8_t read_digital_input(IODINChannel_en channel)
{
	return din_data[channel].din;
}

void write_analogue_output(IODACChannel_en channel, uint32_t value)
{
	dac_data[channel].dac = value;
	update_dac(channel);
}

void write_digital_output(IODOUTChannel channel, uint8_t value)
{
	dout_data[channel].dout = value;
	update_dout(channel);
}

void write_pwm_output(IOPWMChannel_en channel, uint16_t duty, uint32_t freq,
		      uint8_t duty_resolution)
{
	pwm_data[channel].duty = duty;
	pwm_data[channel].frequency = freq;
	pwm_data[channel].duty_resolution = duty_resolution;
	update_pwm(channel);
}

CAN_Msg_t read_can_input(IOCANChannel_en bus, int id)
{
	return {};
}

void write_can_output(IOCANChannel_en bus, int id, CAN_Msg_t msg)
{
}

int spi_read(IOSPIChannel_en channel, uint32_t size, void *buffer)
{
	return 1;
}

int spi_write(IOSPIChannel_en channel, uint32_t size, void *buffer)
{
	return 1;
}

int spi_transfer(IOSPIChannel_en channel, uint32_t size, void *tx_buffer,
		 void *rx_buffer)
{
	return 1;
}

} // namespace IO
} // namespace System
