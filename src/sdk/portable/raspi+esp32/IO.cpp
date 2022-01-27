#include "IO.hpp"
#include "CRC.h"
#include "ECU_Msg.h"
#include "ESP32_Msg.h"
#include "Header.h"
#include "System.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SPI_WAIT_TIME pdMS_TO_TICKS(0)

static const char *const device = "/dev/spidev0.0";
static bool esp32_spi_inited    = false;
static int spi_fd               = -1;

static SemaphoreHandle_t xSemaphore = NULL;
static StaticSemaphore_t xMutexBuffer;

static ALIGN ESP32_In_ADC_t adc_data[9]    = {0};
static ALIGN ESP32_In_Hall_t hall_data[1]  = {0};
static ALIGN ESP32_In_DIN_t din_data[3]    = {0};
static ALIGN ESP32_Out_DAC_t dac_data[2]   = {0};
static ALIGN ESP32_Out_PWM_t pwm_data[2]   = {0};
static ALIGN ESP32_Out_DOUT_t dout_data[1] = {0};

static int init_esp32_spi(void)
{
    static constexpr uint8_t spi_bits_per_word = 8;
    static constexpr uint32_t spi_speed_hz     = 9000000;
    static constexpr uint8_t spi_mode          = SPI_CPHA;

    spi_fd = open(device, O_RDWR);
    if (spi_fd < 0)
    {
        printf("can't open device\n");
        return -1;
    }

    int ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
    if (ret == -1)
    {
        printf("can't set spi mode\n");
        close(spi_fd);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word);
    if (ret == -1)
    {
        printf("can't set bits per word\n");
        close(spi_fd);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz);
    if (ret == -1)
    {
        printf("can't set max speed hz\n");
        close(spi_fd);
        return -1;
    }

    printf("SPI init success\n");

    esp32_spi_inited = true;

    return spi_fd;
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
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        return false;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        return false;
    }

    return true;
}

void update_adc(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_IN_ADC, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    bool ack = receive_ack();
    vTaskDelay(SPI_WAIT_TIME);

    if (!ack) { return; }

    // Receive data
    ESP32_In_ADC_t adc_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_In_ADC_t), &adc_msg);

    if (adc_msg.checksum != calc_crc(&adc_msg, offsetof(ESP32_In_ADC_t, checksum)))
    {
        printf("SPI - Received invalid adc msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    adc_data[channel].adc = adc_msg.adc;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

void update_hall(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_IN_HALL, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    ESP32_In_Hall_t hall_msg = {0};

    System::IO::spi_read(0, sizeof(ESP32_In_Hall_t), &hall_msg);

    if (hall_msg.checksum != calc_crc(&hall_msg, offsetof(ESP32_In_Hall_t, checksum)))
    {
        printf("SPI - Received invalid hall msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    hall_data[channel].hall = hall_msg.hall;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

void update_din(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_IN_DIN, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    ESP32_In_DIN_t din_msg = {0};

    System::IO::spi_read(0, sizeof(ESP32_In_DIN_t), &din_msg);

    if (din_msg.checksum != calc_crc(&din_msg, offsetof(ESP32_In_DIN_t, checksum)))
    {
        printf("SPI - Received invalid hall msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    din_data[channel].din = din_msg.din;

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

void update_dac(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_DAC, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    dac_data[channel].seed     = rand();
    dac_data[channel].checksum = calc_crc(&dac_data[channel], offsetof(ESP32_Out_DAC_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_DAC_t), &dac_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

void update_pwm(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_PWM, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    pwm_data[channel].seed     = rand();
    pwm_data[channel].checksum = calc_crc(&pwm_data[channel], offsetof(ESP32_Out_PWM_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_PWM_t), &pwm_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

void update_dout(int channel)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);

    // Send request
    request_peripheral(ESP32_OUT_DOUT, channel);
    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        printf("SPI - Received invalid acknowledgement msg - bad CRC\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }
    if (ack_msg.type != ESP32_ACK)
    {
        printf("SPI - Received invalid ack command\n");
        vTaskDelay(SPI_WAIT_TIME);
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    dout_data[channel].seed     = rand();
    dout_data[channel].checksum = calc_crc(&dout_data[channel], offsetof(ESP32_Out_DOUT_t, checksum));

    System::IO::spi_write(0, sizeof(ESP32_Out_DOUT_t), &dout_data[channel]);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

REGISTER_ROUTINE(esp_in_update, 60, 128)
{
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
}

REGISTER_ROUTINE(esp_out_update, 1 / 0.025, 128)
{
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
}

namespace System {
namespace IO {

int port_init_io(void)
{
    xSemaphore = xSemaphoreCreateRecursiveMutexStatic(&xMutexBuffer);

    if (!esp32_spi_inited)
    {
        spi_fd = init_esp32_spi();
        if (spi_fd < 0) { return -1; }
    }

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

int spi_read(int channel, uint32_t size, void *buffer) { return spi_transfer(channel, size, nullptr, buffer); }

int spi_write(int channel, uint32_t size, void *buffer) { return spi_transfer(channel, size, buffer, nullptr); }

int spi_transfer(int channel, uint32_t size, void *tx_buffer, void *rx_buffer)
{
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf                  = (__u64)tx_buffer;
    tr.rx_buf                  = (__u64)rx_buffer;
    tr.len                     = size;

    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("can't send spi message\n");
        return -1;
    }

    return 1;
}

} // namespace IO
} // namespace System
