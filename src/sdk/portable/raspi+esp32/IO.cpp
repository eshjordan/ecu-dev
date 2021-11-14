#include "IO.hpp"
#include "ESP32Msg.h"
#include "Message.h"
#include "System.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device    = "/dev/spidev0.0";
static int fd                = -1;
static bool esp32_spi_inited = false;
static int count_val         = 0;

static void init_esp32_spi(void)
{
    static constexpr uint8_t spi_bits_per_word = 8;
    static constexpr uint32_t spi_speed_hz     = 500000;
    static constexpr uint8_t spi_mode          = SPI_CPHA;

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        puts("can't open device");
        return;
    }

    int ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_mode);
    if (ret == -1)
    {
        puts("can't set spi mode");
        return;
    }

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word);
    if (ret == -1)
    {
        puts("can't set bits per word");
        return;
    }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz);
    if (ret == -1)
    {
        puts("can't set max speed hz");
        return;
    }

    puts("SPI init success");

    esp32_spi_inited = true;
}

REGISTER_ROUTINE(esp_get_status, 1)
{
    if (!esp32_spi_inited)
    {
        puts("io not inited");
        init_esp32_spi();
    }
    puts("esp_get_status");

    __attribute__((aligned(4))) static uint8_t tx_buf[3 * 112] = {0};
    __attribute__((aligned(4))) static uint8_t rx_buf[3 * 112] = {0};

    static const Message_t *const rx_msg = (Message_t *)rx_buf;

    static struct spi_ioc_transfer tr = {0};
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (__u64)&tx_buf;
    tr.rx_buf = (__u64)&rx_buf;
    tr.len    = 3 * sizeof(Message_t);

    static char msg[1024] = {0};

    static Message_t request_msg = {};
    uint64_t zero                = 0;
    make_message(&request_msg, 0, "spi_request", &zero, Message_t::STATUS_CMD);

    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    memcpy(tx_buf, &request_msg, sizeof(request_msg));

    if (fd < 0)
    {
        puts("spi not open");
        return;
    }

    // for (uint8_t i = 0; i < sizeof(Message_t); i++)
    // {
    //     tx_buf[i] = i;
    // }

    // puts("SENDING:");
    // for (int i = 0; i < sizeof(Message_t); i++)
    // {
    //     printf("%hu\n", tx_buf[i]);
    // }

    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        puts("can't send spi message");
        return;
    }

    // puts("RECEIVING:");
    // for (int i = 0; i < sizeof(Message_t); i++)
    // {
    //     printf("%hu\n", rx_buf[i]);
    // }

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
    make_message(&ack_msg, 0, "spi_ack", &zero, Message_t::ACK_CMD);

    // Copy the IO state to the message buffer, clear rx buffer
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    memcpy(tx_buf, &ack_msg, sizeof(ack_msg));

    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (__u64)&tx_buf;
    tr.rx_buf = (__u64)&rx_buf;
    tr.len    = 3 * sizeof(Message_t);

    vTaskDelay(pdMS_TO_TICKS(100));

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        puts("can't send spi message");
        goto nd;
    }

    memset(msg, 0, sizeof(msg));
    ESP32Msg_to_str(msg, (ESP32Msg_t *)rx_buf);
    printf("Rx Msg:\n%s\n", msg);

nd:
    puts("");
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
