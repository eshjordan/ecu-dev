#include "IO.hpp"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

namespace System {
namespace IO {

int port_init_io(void) { return 1; }

uint32_t read_analogue_input(int channel) { return 0; }

uint32_t read_hall_input(int channel) { return 0; }

uint8_t read_digital_input(int channel) { return 0; }

void write_analogue_output(int channel, uint32_t value) {}

void write_digital_output(int channel, uint8_t value) {}

void write_pwm_output(int channel, uint16_t duty, uint32_t freq, uint8_t duty_resolution) {}

CAN_Msg_t read_can_input(int bus, int id) { return {}; }

void write_can_output(int bus, int id, CAN_Msg_t msg) {}

int spi_read(int channel, uint32_t size, void *buffer) { return spi_transfer(channel, size, nullptr, buffer); }

int spi_write(int channel, uint32_t size, void *buffer) { return spi_transfer(channel, size, buffer, nullptr); }

int spi_transfer(int channel, uint32_t size, void *tx_buffer, void *rx_buffer) { return 1; }

} // namespace IO
} // namespace System
