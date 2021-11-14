#include "IO.hpp"
#include "ESP32Msg.h"
#include "Message.h"
#include "System.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *const device = "/dev/spidev0.0";
static bool esp32_spi_inited    = false;
static int spi_fd               = -1;

static ESP32Msg_t esp_status = {};

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

REGISTER_ROUTINE(esp_get_status, 1)
{
    __attribute__((aligned(4))) static uint8_t tx_buf[1024] = {0};
    __attribute__((aligned(4))) static uint8_t rx_buf[1024] = {0};

    static const auto *const rx_msg = (const Message_t *const)rx_buf;

    static char msg[1024] = {0};
    int esp_ret           = -1;

    static Message_t request_msg = {};
    request_msg                  = make_message(0, "spi_request", nullptr, Message_t::STATUS_CMD);

    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    memcpy(tx_buf, &request_msg, sizeof(request_msg));

    System::IO::spi_transfer(0, sizeof(Message_t), tx_buf, rx_buf);

    memset(msg, 0, sizeof(msg));
    msg_to_str(msg, rx_msg);
    printf("SPI - Message before flipping:\n%s\n", msg);

    int msg_status = check_msg(rx_msg);
    if (msg_status < 0)
    {
        memset(msg, 0, sizeof(msg));
        msg_err_to_str(msg, msg_status);
        printf("SPI - Received invalid acknowledgement msg - %s\n", msg);

        memset(msg, 0, sizeof(msg));
        msg_to_str(msg, rx_msg);
        printf("SPI - Message:\n%s\n", msg);
        goto nd;
    }

    if (rx_msg->command != Message_t::ACK_CMD)
    {
        memset(msg, 0, sizeof(msg));
        msg_to_str(msg, rx_msg);
        printf("SPI - Received invalid ack command - %s\n", msg);
        goto nd;
    }

    static Message_t ack_msg = {};
    ack_msg                  = make_message(0, "spi_ack", nullptr, Message_t::ACK_CMD);

    // Copy the IO state to the message buffer, clear rx buffer
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    memcpy(tx_buf, &ack_msg, sizeof(ack_msg));

    vTaskDelay(pdMS_TO_TICKS(100));

    System::IO::spi_transfer(0, std::max(sizeof(ESP32Msg_t), sizeof(Message_t)), tx_buf, rx_buf);

    memset(msg, 0, sizeof(msg));
    ESP32Msg_to_str(msg, (ESP32Msg_t *)rx_buf);
    printf("Rx Msg:\n%s\n", msg);

    esp_ret = check_esp((ESP32Msg_t *)rx_buf);
    if (esp_ret < 0)
    {
        esp_err_to_str(msg, esp_ret);
        printf("ESP Msg error: %s\n", msg);
        goto nd;
    }

    memcpy(&esp_status, rx_buf, sizeof(ESP32Msg_t));

nd:
    puts("");
}

namespace System {
namespace IO {

double read_analogue_input(int channel) { return esp_status.adc[channel]; }

void write_analogue_output(int channel, double value) {}

int read_digital_input(int channel)
{
    switch (channel)
    {
    case 0: {
        return esp_status.din1;
    }
    case 1: {
        return esp_status.din2;
    }
    case 2: {
        return esp_status.din3;
    }
    default: {
        return 0;
    }
    }
}

void write_digital_output(int channel, int value) {}

void write_pwm_output(int channel, double freq, double duty) {}

CANMsg_t read_can_input(int bus, int id) { return esp_status.CANMsg; }

void write_can_output(int bus, int id, CANMsg_t msg) {}

int spi_read(int channel, int size, uint8_t *buffer) { return spi_transfer(channel, size, nullptr, buffer); }

int spi_write(int channel, int size, uint8_t *buffer) { return spi_transfer(channel, size, buffer, nullptr); }

int spi_transfer(int channel, int size, uint8_t *tx_buffer, uint8_t *rx_buffer)
{
    if (!esp32_spi_inited)
    {
        puts("io not inited");
        spi_fd = init_esp32_spi();
        if (spi_fd < 0) { return -1; }
    }

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
