#include "IO.hpp"
#include "CRC.h"
#include "ECU_Msg.h"
#include "ESP32_Msg.h"
#include "Header.h"
#include "System.hpp"
#include "main.h"
#include "stm32System.h"
#include "cmsis_os.h"
#include "stm32mp1xx_hal_spi.h"
#include "stm32System.h"

extern "C" {
#include "CANSPI.h"
}

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

extern osMutexId_t esp32DataMutexHandle;

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
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

	// Send request
	request_peripheral(ESP32_IN_ADC, channel);
	vTaskDelay(SPI_WAIT_TIME);

	// Receive acknowledgement
	bool ack = receive_ack();
	vTaskDelay(SPI_WAIT_TIME);

	if (!ack) {
		osMutexRelease(esp32DataMutexHandle);
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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	adc_data[channel].adc = adc_msg.adc;

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
}

void update_hall(IOHallChannel channel)
{
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		osMutexRelease(esp32DataMutexHandle);
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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	hall_data[channel].hall = hall_msg.hall;

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
}

void update_din(IODINChannel_en channel)
{
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		osMutexRelease(esp32DataMutexHandle);
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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	din_data[channel].din = din_msg.din;

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
}

void update_dac(IODACChannel_en channel)
{
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	dac_data[channel].seed = rand();
	dac_data[channel].checksum = calc_crc(
		&dac_data[channel], offsetof(ESP32_Out_DAC_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_DAC_t),
			      &dac_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
}

void update_pwm(IOPWMChannel_en channel)
{
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	pwm_data[channel].seed = rand();
	pwm_data[channel].checksum = calc_crc(
		&pwm_data[channel], offsetof(ESP32_Out_PWM_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_PWM_t),
			      &pwm_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
}

void update_dout(IODOUTChannel channel)
{
	osMutexAcquire(esp32DataMutexHandle, portMAX_DELAY);

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
		osMutexRelease(esp32DataMutexHandle);
		return;
	}
	if (ack_msg.type != ESP32_ACK) {
		printf("SPI - Received invalid ack command\n");
		vTaskDelay(SPI_WAIT_TIME);
		osMutexRelease(esp32DataMutexHandle);
		return;
	}

	vTaskDelay(SPI_WAIT_TIME);

	dout_data[channel].seed = rand();
	dout_data[channel].checksum = calc_crc(
		&dout_data[channel], offsetof(ESP32_Out_DOUT_t, checksum));

	System::IO::spi_write(IO_SPI_CHANNEL_01, sizeof(ESP32_Out_DOUT_t),
			      &dout_data[channel]);

	vTaskDelay(SPI_WAIT_TIME);

	osMutexRelease(esp32DataMutexHandle);
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

static CAN_Msg_t ucanmsg_to_can_msg(uCAN_MSG *msg)
{
	CAN_Msg_t msg_out;

	msg_out.header = header_make(0, sizeof(CAN_Msg_t));
	msg_out.identifier = msg->frame.id;
	msg_out.data_length_code = msg->frame.dlc;

	msg_out.data[0] = msg->frame.data0;
	msg_out.data[1] = msg->frame.data1;
	msg_out.data[2] = msg->frame.data2;
	msg_out.data[3] = msg->frame.data3;
	msg_out.data[4] = msg->frame.data4;
	msg_out.data[5] = msg->frame.data5;
	msg_out.data[6] = msg->frame.data6;
	msg_out.data[7] = msg->frame.data7;

	can_msg_calc_checksum(&msg_out);
	return msg_out;
}

static uCAN_MSG can_msg_to_ucanmsg(CAN_Msg_t *msg)
{
	uCAN_MSG msg_out = { .frame = { .id = msg->identifier,
					.dlc = msg->data_length_code,
					.data0 = msg->data[0],
					.data1 = msg->data[1],
					.data2 = msg->data[2],
					.data3 = msg->data[3],
					.data4 = msg->data[4],
					.data5 = msg->data[5],
					.data6 = msg->data[6],
					.data7 = msg->data[7] } };

	return msg_out;
}

uint32_t can_msg_get_dlc(CAN_Msg_t *msg)
{
	uint32_t num_bytes = FDCAN_DLC_BYTES_8;

	switch (msg->data_length_code) {
	case 0:
		num_bytes = FDCAN_DLC_BYTES_0;
		break;
	case 1:
		num_bytes = FDCAN_DLC_BYTES_1;
		break;
	case 2:
		num_bytes = FDCAN_DLC_BYTES_2;
		break;
	case 3:
		num_bytes = FDCAN_DLC_BYTES_3;
		break;
	case 4:
		num_bytes = FDCAN_DLC_BYTES_4;
		break;
	case 5:
		num_bytes = FDCAN_DLC_BYTES_5;
		break;
	case 6:
		num_bytes = FDCAN_DLC_BYTES_6;
		break;
	case 7:
		num_bytes = FDCAN_DLC_BYTES_7;
		break;
	case 8:
		num_bytes = FDCAN_DLC_BYTES_8;
		break;
	case 12:
		num_bytes = FDCAN_DLC_BYTES_12;
		break;
	case 16:
		num_bytes = FDCAN_DLC_BYTES_16;
		break;
	case 20:
		num_bytes = FDCAN_DLC_BYTES_20;
		break;
	case 24:
		num_bytes = FDCAN_DLC_BYTES_24;
		break;
	case 32:
		num_bytes = FDCAN_DLC_BYTES_32;
		break;
	case 48:
		num_bytes = FDCAN_DLC_BYTES_48;
		break;
	case 64:
		num_bytes = FDCAN_DLC_BYTES_64;
		break;
	default:
		num_bytes = FDCAN_DLC_BYTES_8;
		break;
	}

	return num_bytes;
}

uint8_t fdcan_msg_get_dlc(FDCAN_RxHeaderTypeDef *msg)
{
	uint8_t num_bytes = 8;

	switch (msg->DataLength) {
	case FDCAN_DLC_BYTES_0:
		num_bytes = 0;
		break;
	case FDCAN_DLC_BYTES_1:
		num_bytes = 1;
		break;
	case FDCAN_DLC_BYTES_2:
		num_bytes = 2;
		break;
	case FDCAN_DLC_BYTES_3:
		num_bytes = 3;
		break;
	case FDCAN_DLC_BYTES_4:
		num_bytes = 4;
		break;
	case FDCAN_DLC_BYTES_5:
		num_bytes = 5;
		break;
	case FDCAN_DLC_BYTES_6:
		num_bytes = 6;
		break;
	case FDCAN_DLC_BYTES_7:
		num_bytes = 7;
		break;
	case FDCAN_DLC_BYTES_8:
		num_bytes = 8;
		break;
	case FDCAN_DLC_BYTES_12:
		num_bytes = 12;
		break;
	case FDCAN_DLC_BYTES_16:
		num_bytes = 16;
		break;
	case FDCAN_DLC_BYTES_20:
		num_bytes = 20;
		break;
	case FDCAN_DLC_BYTES_24:
		num_bytes = 24;
		break;
	case FDCAN_DLC_BYTES_32:
		num_bytes = 32;
		break;
	case FDCAN_DLC_BYTES_48:
		num_bytes = 48;
		break;
	case FDCAN_DLC_BYTES_64:
		num_bytes = 64;
		break;
	default:
		num_bytes = 8;
		break;
	}

	return num_bytes;
}

CAN_Msg_t read_can_input(IOCANChannel_en bus, int id)
{
	uint8_t success = 0;
	CAN_Msg_t msg;

	uint32_t err_status = HAL_FDCAN_GetError(can_bus_01_handle);
	HAL_FDCAN_StateTypeDef state_status =
		HAL_FDCAN_GetState(can_bus_01_handle);

	if (state_status == HAL_FDCAN_STATE_RESET) {
		success = HAL_ERROR;
		log_info("CAN is in reset state");
		return {};
	}

	if (state_status == HAL_FDCAN_STATE_ERROR) {
		success = HAL_ERROR;
		log_warn("CAN RX error");
		return {};
	}

	switch (bus) {
	case IO_CAN_CHANNEL_01: {
		uint32_t fill = HAL_FDCAN_GetRxFifoFillLevel(can_bus_01_handle,
							     FDCAN_RX_FIFO0);
		if (fill == 0) {
			success = HAL_ERROR;
			log_warn("RX FIFO 0 is empty: %d", fill);
			break;
		}

		FDCAN_RxHeaderTypeDef pRxHeader;
		success = HAL_FDCAN_GetRxMessage(can_bus_01_handle,
						 FDCAN_RX_FIFO0, &pRxHeader,
						 msg.data);
		if (success != HAL_OK) {
			success = HAL_ERROR;
			log_err("Not ok on CAN RX! %d", success);
		}

		msg.identifier = pRxHeader.Identifier;
		msg.data_length_code = fdcan_msg_get_dlc(&pRxHeader);

		break;
	}
	case IO_CAN_CHANNEL_02:
	case IO_CAN_CHANNEL_03:
	default: {
		break;
	}
	}

	return msg;
}

void write_can_output(IOCANChannel_en bus, int id, CAN_Msg_t msg)
{
	HAL_StatusTypeDef success = HAL_OK;

	uint32_t err_status = HAL_FDCAN_GetError(can_bus_01_handle);
	HAL_FDCAN_StateTypeDef state_status =
		HAL_FDCAN_GetState(can_bus_01_handle);

	if (state_status == HAL_FDCAN_STATE_RESET) {
		success = HAL_ERROR;
		log_info("CAN is in reset state");
		return;
	}

	if (state_status == HAL_FDCAN_STATE_ERROR) {
		success = HAL_ERROR;
		log_warn("CAN TX error");
		return;
	}

	FDCAN_TxHeaderTypeDef pTxHeader;
	pTxHeader.IdType = FDCAN_STANDARD_ID;
	pTxHeader.Identifier = msg.identifier;
    pTxHeader.DataLength = can_msg_get_dlc(&msg);
	pTxHeader.TxFrameType = FDCAN_DATA_FRAME;
	pTxHeader.FDFormat = FDCAN_CLASSIC_CAN;

	uint32_t freeLevel = HAL_FDCAN_GetTxFifoFreeLevel(can_bus_01_handle);

	switch (bus) {
	case IO_CAN_CHANNEL_01: {
		HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(
			can_bus_01_handle, &pTxHeader, msg.data);
		if (status != HAL_OK) {
			success = HAL_ERROR;
			log_err("Not ok on CAN TX! %d", status);
		}
		break;
	}
	case IO_CAN_CHANNEL_02:
	case IO_CAN_CHANNEL_03:
	default: {
		break;
	}
	}
}

int spi_read(IOSPIChannel_en channel, uint32_t size, void *buffer)
{
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */

	switch (channel) {
	case IO_SPI_CHANNEL_01: {
		if (HAL_SPI_Receive(esp32_spi_handle, (uint8_t *)buffer, size,
				    100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_02: {
		if (HAL_SPI_Receive(mcp2515_spi_handle, (uint8_t *)buffer, size,
				    100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_03:
	default: {
		return -1;
	}
	}

	return 1;
}

int spi_write(IOSPIChannel_en channel, uint32_t size, void *buffer)
{
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */

	switch (channel) {
	case IO_SPI_CHANNEL_01: {
		if (HAL_SPI_Transmit(esp32_spi_handle, (uint8_t *)buffer, size,
				     100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_02: {
		if (HAL_SPI_Transmit(mcp2515_spi_handle, (uint8_t *)buffer,
				     size, 100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_03:
	default: {
		return -1;
	}
	}

	return 1;
}

int spi_transfer(IOSPIChannel_en channel, uint32_t size, void *tx_buffer,
		 void *rx_buffer)
{
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */

	switch (channel) {
	case IO_SPI_CHANNEL_01: {
		if (HAL_SPI_TransmitReceive(
			    esp32_spi_handle, (uint8_t *)tx_buffer,
			    (uint8_t *)rx_buffer, size, 100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_02: {
		if (HAL_SPI_TransmitReceive(
			    mcp2515_spi_handle, (uint8_t *)tx_buffer,
			    (uint8_t *)rx_buffer, size, 100) != HAL_OK) {
			/* Transfer error in transmission process */
			return -1;
		}
		break;
	}
	case IO_SPI_CHANNEL_03:
	default: {
		return -1;
	}
	}

	return 1;
}

} // namespace IO
} // namespace System
