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

#define SPI_WAIT_TIME pdMS_TO_TICKS(2)

static const char *const device = "/dev/spidev0.0";
static bool esp32_spi_inited    = false;
static int spi_fd               = -1;

static SemaphoreHandle_t xSemaphore = NULL;
static StaticSemaphore_t xMutexBuffer;

// static ESP32_In_Msg_t esp_status  = {};
// static ESP32_Out_Msg_t esp_output = {};

static int init_esp32_spi(void)
{
    static constexpr uint8_t spi_bits_per_word = 8;
    static constexpr uint32_t spi_speed_hz     = 9000000;
    static constexpr uint8_t spi_mode          = SPI_CPHA;

    spi_fd = open(device, O_RDWR);
    if (spi_fd < 0)
    {
        puts("can't open device");
        return -1;
    }

    int ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
    if (ret == -1)
    {
        puts("can't set spi mode");
        close(spi_fd);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word);
    if (ret == -1)
    {
        puts("can't set bits per word");
        close(spi_fd);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz);
    if (ret == -1)
    {
        puts("can't set max speed hz");
        close(spi_fd);
        return -1;
    }

    puts("SPI init success");

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

REGISTER_ROUTINE(esp_get_status, 60)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);
    // Send request
    request_peripheral(ESP32_IN_ADC, 0);

    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        return;
    }

    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    ALIGN ESP32_In_ADC_t adc = {0};
    System::IO::spi_read(0, sizeof(ESP32_In_ADC_t), &adc);

    if (adc.checksum != calc_crc(&adc, offsetof(ESP32_In_ADC_t, checksum)))
    {
        puts("SPI - Received invalid adc msg - bad CRC");
        return;
    }

    std::cout << "ADC: " << adc.adc << "\n";

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
}

REGISTER_ROUTINE(esp_pwm, 60)
{
    xSemaphoreTakeRecursive(xSemaphore, portMAX_DELAY);
    // Send request
    request_peripheral(ESP32_OUT_PWM, 0);

    vTaskDelay(SPI_WAIT_TIME);

    // Receive acknowledgement
    ALIGN ESP32_Request_t ack_msg = {0};
    System::IO::spi_read(0, sizeof(ESP32_Request_t), &ack_msg);

    if (ack_msg.checksum != calc_crc(&ack_msg, offsetof(ESP32_Request_t, checksum)))
    {
        puts("SPI - Received invalid acknowledgement msg - bad CRC");
        return;
    }

    if (ack_msg.type != ESP32_ACK)
    {
        puts("SPI - Received invalid ack command");
        return;
    }

    vTaskDelay(SPI_WAIT_TIME);

    static int count          = 0;
    ALIGN ESP32_Out_PWM_t pwm = {0};
    pwm.seed                  = rand();
    pwm.duty_resolution       = 8;
    pwm.frequency             = 5000;
    pwm.duty                  = count++;
    pwm.checksum              = calc_crc(&pwm, offsetof(ESP32_Out_PWM_t, checksum));
    if (count >= 255) { count = 0; }
    System::IO::spi_write(0, sizeof(ESP32_Out_PWM_t), &pwm);

    vTaskDelay(SPI_WAIT_TIME);

    xSemaphoreGiveRecursive(xSemaphore);
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

double read_analogue_input(int channel) { return 0; }

void write_analogue_output(int channel, double value) {}

int read_digital_input(int channel)
{
    switch (channel)
    {
    case 0: {
        // return esp_status.din[0];
    }
    case 1: {
        // return esp_status.din[1];
    }
    case 2: {
        // return esp_status.din[2];
    }
    default: {
        return 0;
    }
    }
}

void write_digital_output(int channel, int value) {}

void write_pwm_output(int channel, double freq, double duty) {}

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
        puts("can't send spi message");
        return -1;
    }

    return 1;
}

} // namespace IO
} // namespace System
