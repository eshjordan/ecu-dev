#include "CANMsg.h"

namespace System {
namespace IO {

double read_analogue_input(int channel);

void write_analogue_output(int channel, double value);

int read_digital_input(int channel);

void write_digital_output(int channel, int value);

void write_pwm_output(int channel, double freq, double duty);

CANMsg_t read_can_input(int bus, int id);

void write_can_output(int bus, int id, CANMsg_t msg);

int spi_read(int channel, int size, uint8_t *buffer);

int spi_write(int channel, int size, uint8_t *buffer);

int spi_transfer(int channel, int size, uint8_t *tx_buffer, uint8_t *rx_buffer);

} // namespace IO
} // namespace System
