#include "IO.hpp"
#include "ESP32_In_Msg.h"
#include "ECU_Msg.h"
#include "System.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *const device = "/dev/spidev0.0";
static bool esp32_spi_inited    = false;
static int spi_fd               = -1;

static ESP32_In_Msg_t esp_status = {};

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

REGISTER_ROUTINE(esp_get_status, 10)
{
#define print_status 0

    static __attribute__((aligned(4))) ECU_Msg_t tx_msg = {};
    static __attribute__((aligned(4))) ECU_Msg_t rx_msg = {};

    static char msg[1024] = {0};
    int esp_ret           = -1;

    memset(&rx_msg, 0, sizeof(ECU_Msg_t));

    tx_msg = ecu_msg_make(0, "spi_request", nullptr, ECU_Msg_t::STATUS_CMD);

    System::IO::spi_transfer(0, sizeof(ECU_Msg_t), &tx_msg, &rx_msg);

#if print_status
    memset(msg, 0, sizeof(msg));
    msg_to_str(msg, &rx_msg);
    printf("SPI - Message before flipping:\n%s\n", msg);
#endif

    int msg_status = ecu_msg_check(&rx_msg);
    if (msg_status < 0)
    {
        memset(msg, 0, sizeof(msg));
        ecu_err_to_str(msg, msg_status);
        printf("SPI - Received invalid acknowledgement msg - %s\n", msg);
        goto nd;
    }

    if (rx_msg.command != ECU_Msg_t::ACK_CMD)
    {
        memset(msg, 0, sizeof(msg));
        ecu_msg_to_str(msg, &rx_msg);
        printf("SPI - Received invalid ack command - %s\n", msg);
        goto nd;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    // Copy the IO state to the message buffer, clear rx buffer
    memset(&rx_msg, 0, sizeof(ECU_Msg_t));

    tx_msg = ecu_msg_make(0, "spi_ack", nullptr, ECU_Msg_t::ACK_CMD);

    System::IO::spi_transfer(0, std::max(sizeof(ESP32_In_Msg_t), sizeof(ECU_Msg_t)), &tx_msg, &esp_status);

#if print_status
    memset(msg, 0, sizeof(msg));
    esp32_in_msg_to_str(msg, &esp_status);
    printf("Rx Msg:\n%s\n", msg);
#endif

    esp_ret = esp32_in_msg_check(&esp_status);
    if (esp_ret < 0)
    {
        ecu_err_to_str(msg, esp_ret);
        printf("ESP Msg error: %s\n", msg);
        goto nd;
    }

nd:
    __asm("nop");
}

// REGISTER_ROUTINE(esp_print_status, 1)
// {
//     for (int i = 0; i < 9; i++)
//     {
//         printf("adc %d: %lf\n", i, System::IO::read_analogue_input(i));
//     }

//     for (int i = 0; i < 3; i++)
//     {
//         printf("din %d: %d\n", i, System::IO::read_digital_input(i));
//     }
// }

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

CAN_Msg_t read_can_input(int bus, int id) { return esp_status.can_msg; }

void write_can_output(int bus, int id, CAN_Msg_t msg) {}

int spi_read(int channel, int size, void *buffer) { return spi_transfer(channel, size, nullptr, buffer); }

int spi_write(int channel, int size, void *buffer) { return spi_transfer(channel, size, buffer, nullptr); }

int spi_transfer(int channel, int size, void *tx_buffer, void *rx_buffer)
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
