#include "CAN_Msg.h"

namespace System {
namespace IO {

double read_analogue_input(int channel);

void write_analogue_output(int channel, double value);

int read_digital_input(int channel);

void write_digital_output(int channel, int value);

void write_pwm_output(int channel, double freq, double duty);

CAN_Msg_t read_can_input(int bus, int id);

void write_can_output(int bus, int id, CAN_Msg_t msg);

int spi_read(int channel, uint32_t size, void *buffer);

int spi_write(int channel, uint32_t size, void *buffer);

int spi_transfer(int channel, uint32_t size, void *tx_buffer, void *rx_buffer);

} // namespace IO
} // namespace System
