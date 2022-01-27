#include "IO.hpp"
#include "ECU_Msg.h"
#include "System.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev0.0";
static int fd             = -1;
static bool io_inited     = false;

static void transfer()
{
    ECU_Msg_t tx_msg = {};

    ECU_Msg_t rx_msg = {};

    struct spi_ioc_transfer tr = {.tx_buf = (unsigned long)&tx_msg, .rx_buf = (unsigned long)&rx_msg, .len = sizeof(ECU_Msg_t)};

    if (fd < 0)
    {
        printf("spi not open\n");
        return;
    }

    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("can't send spi message\n");
        return;
    }

    printf("spi message:\n");

    char msg_str[128] = {};

    msg_to_str(msg_str, &rx_msg);
    printf("%s\n", msg_str);

    printf("\n");
}

int port_init_io(void)
{
    static constexpr uint8_t spi_bits_per_word = 8;
    static constexpr uint32_t spi_speed_hz     = 125000;

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        printf("can't open device\n");
        return 0;
    }

    int ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word);
    if (ret == -1)
    {
        printf("can't set bits per word\n");
        return 0;
    }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz);
    if (ret == -1)
    {
        printf("can't set max speed hz\n");
        return 0;
    }

    io_inited = true;
}

static int count_val = 0;

REGISTER_ROUTINE(esp_get_status, 1, 128)
{
    if (!io_inited) {
        printf("io not inited\n");
        port_init_io(nullptr);
    }
    printf("esp_get_status\n");
    transfer();
}

namespace System {
namespace IO {

double read_analogue_input(int channel) { return count_val; }

void write_analogue_output(int channel, double value) {}

int read_digital_input(int channel) { return count_val; }

void write_digital_output(int channel, int value) {}

void write_pwm_output(int channel, double freq, double duty) {}

CANMsg_t read_can_input(int bus, int id) { return {}; }

void write_can_output(int bus, int id, CANMsg_t msg) {}

void read_spi_input(int channel, int size, char *data) {}

void write_spi_input(int channel, int size, char *data) {}

} // namespace IO
} // namespace System
