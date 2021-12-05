#include "IO.hpp"
#include "CRC.h"
#include "ECU_Msg.h"
#include "ESP32_Msg.h"
#include "Header.h"
#include "System.hpp"
#include "main.h"
#include "cmsis_os.h"
#include "stm32mp1xx_hal_spi.h"
#include "stm32.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SPI_WAIT_TIME pdMS_TO_TICKS(1)

static TaskHandle_t in_update_handle = {};
static TaskHandle_t out_update_handle = {};

static StackType_t in_update_stack[4] = {};
static StackType_t out_update_stack[4] = {};

static StaticTask_t in_update_tsk_buf = {};
static StaticTask_t out_update_tsk_buf = {};

static bool esp32_spi_inited    = false;

extern SPI_HandleTypeDef hspi5;

extern osMutexId esp32DataMutexHandle;

static ALIGN ESP32_In_ADC_t adc_data[9]    = {0};
static ALIGN ESP32_In_Hall_t hall_data[1]  = {0};
static ALIGN ESP32_In_DIN_t din_data[3]    = {0};
static ALIGN ESP32_Out_DAC_t dac_data[2]   = {0};
static ALIGN ESP32_Out_PWM_t pwm_data[2]   = {0};
static ALIGN ESP32_Out_DOUT_t dout_data[1] = {0};

static int init_esp32_spi(void)
{
    puts("SPI init success");

    esp32_spi_inited = true;

    return 1;
}

void request_peripheral(uint8_t type, uint8_t channel)
{
    ALIGN ESP32_Request_t rx_buf = {0};
    ALIGN ESP32_Request_t tx_msg = {0};
    tx_msg.seed                  = rand();
    tx_msg.type                  = type;
    tx_msg.channel               = channel;
    tx_msg.checksum              = calc_crc(&tx_msg, offsetof(ESP32_Request_t, checksum));

    System::IO::spi_transfer(0, sizeof(ESP32_Request_t), &tx_msg, &rx_buf);
}

bool receive_ack(void)
{
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        return false;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        return false;
    }

    return true;
}

void update_adc(int channel)
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
    ESP32_In_ADC_t adc_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_In_ADC_t), &adc_msg);

    if (adc_msg.checksum != calc_crc(&adc_msg, offsetof(ESP32_In_ADC_t, checksum)))
    {
        puts("SPI - Received invalid adc msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    adc_data[channel].adc = adc_msg.adc;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

void update_hall(int channel)
{
    xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_IN_HALL, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    ESP32_In_Hall_t hall_msg = {0};

    System::IO::spi_read(0, sizeof(ESP32_In_Hall_t), &hall_msg);

    if (hall_msg.checksum != calc_crc(&hall_msg, offsetof(ESP32_In_Hall_t, checksum)))
    {
        puts("SPI - Received invalid hall msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    hall_data[channel].hall = hall_msg.hall;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

void update_din(int channel)
{
    xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_IN_DIN, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    ESP32_In_DIN_t din_msg = {0};

    System::IO::spi_read(0, sizeof(ESP32_In_DIN_t), &din_msg);

    if (din_msg.checksum != calc_crc(&din_msg, offsetof(ESP32_In_DIN_t, checksum)))
    {
        puts("SPI - Received invalid hall msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    din_data[channel].din = din_msg.din;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

void update_dac(int channel)
{
    xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_DAC, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    dac_data[channel].seed     = rand();
    dac_data[channel].checksum = calc_crc(&dac_data[channel], offsetof(ESP32_Out_DAC_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_DAC_t), &dac_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

void update_pwm(int channel)
{
    xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_PWM, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    pwm_data[channel].seed     = rand();
    pwm_data[channel].checksum = calc_crc(&pwm_data[channel], offsetof(ESP32_Out_PWM_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_PWM_t), &pwm_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

void update_dout(int channel)
{
    xSemaphoreTake(esp32DataMutexHandle, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_DOUT, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        vTaskDelay(SPI_WAIT_TIME);
        xSemaphoreGive(esp32DataMutexHandle);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    dout_data[channel].seed     = rand();
    dout_data[channel].checksum = calc_crc(&dout_data[channel], offsetof(ESP32_Out_DOUT_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_DOUT_t), &dout_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGive(esp32DataMutexHandle);
}

extern "C" void esp_in_update(TimerHandle_t handle)
{
//	while(1)
//	{
		// Inputs
		for (int i = 0; i < ARRAY_SIZE(adc_data); i++)
		{
			update_adc(i);
		}

		for (int i = 0; i < ARRAY_SIZE(hall_data); i++)
		{
			update_hall(i);
		}

		for (int i = 0; i < ARRAY_SIZE(din_data); i++)
		{
			update_din(i);
		}

//		vTaskDelay(pdMS_TO_TICKS(16));
//	}
}

extern "C" void esp_out_update(TimerHandle_t handle)
{
//	while(1)
//	{
		// Outputs
		for (int i = 0; i < ARRAY_SIZE(dac_data); i++)
		{
			update_dac(i);
		}

		for (int i = 0; i < ARRAY_SIZE(pwm_data); i++)
		{
			update_pwm(i);
		}

		for (int i = 0; i < ARRAY_SIZE(dout_data); i++)
		{
			update_dout(i);
		}

//		vTaskDelay(pdMS_TO_TICKS(16));
//	}
}

namespace System {
namespace IO {

int port_init_io(void)
{
//    xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);

    if (!esp32_spi_inited)
    {
        if (init_esp32_spi() < 0) { return -1; }
    }

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

uint32_t read_analogue_input(int channel) { return adc_data[channel].adc; }

uint32_t read_hall_input(int channel) { return hall_data[channel].hall; }

uint8_t read_digital_input(int channel) { return din_data[channel].din; }

void write_analogue_output(int channel, uint32_t value)
{
    dac_data[channel].dac = value;
    update_dac(channel);
}

void write_digital_output(int channel, uint8_t value)
{
    dout_data[channel].dout = value;
    update_dout(channel);
}

void write_pwm_output(int channel, uint16_t duty, uint32_t freq, uint8_t duty_resolution)
{
    pwm_data[channel].duty            = duty;
    pwm_data[channel].frequency       = freq;
    pwm_data[channel].duty_resolution = duty_resolution;
    update_pwm(channel);
}

CAN_Msg_t read_can_input(int bus, int id) { return {}; }

void write_can_output(int bus, int id, CAN_Msg_t msg) {}

int spi_read(int channel, uint32_t size, void *buffer) {
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */
	if(HAL_SPI_Receive(&hspi5, (uint8_t*)buffer, size, 100) != HAL_OK)
	{
	  /* Transfer error in transmission process */
	  return -1;
	}

	return 1;
}

int spi_write(int channel, uint32_t size, void *buffer) {
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */
	if(HAL_SPI_Transmit(&hspi5, (uint8_t*)buffer, size, 100) != HAL_OK)
	{
	  /* Transfer error in transmission process */
		return -1;
	}

	return 1;
}

int spi_transfer(int channel, uint32_t size, void *tx_buffer, void *rx_buffer)
{
	/*##-1- Start the Full Duplex Communication process ########################*/
	/* While the SPI in TransmitReceive process, user can transmit data through
	   "aTxBuffer" buffer & receive data through "aRxBuffer" */
	if(HAL_SPI_TransmitReceive(&hspi5, (uint8_t*)tx_buffer, (uint8_t *)&rx_buffer, size, 100) != HAL_OK)
	{
	  /* Transfer error in transmission process */
	  return -1;
	}

    return 1;
}

} // namespace IO
} // namespace System
