#include "IOTypes.h"
#include "CAN_Msg.h"

namespace System {
namespace IO {

int port_init_io(void);

uint32_t read_analogue_input(IOADCChannel_en channel);

uint32_t read_hall_input(IOHallChannel channel);

uint8_t read_digital_input(IODINChannel_en channel);

void write_analogue_output(IODACChannel_en channel, uint32_t value);

void write_digital_output(IODOUTChannel channel, uint8_t value);

void write_pwm_output(IOPWMChannel_en channel, uint16_t duty, uint32_t freq, uint8_t duty_resolution);

CAN_Msg_t read_can_input(IOCANChannel_en bus, int id);

void write_can_output(IOCANChannel_en bus, int id, CAN_Msg_t msg);

int spi_read(IOSPIChannel_en channel, uint32_t size, void *buffer);

int spi_write(IOSPIChannel_en channel, uint32_t size, void *buffer);

int spi_transfer(IOSPIChannel_en channel, uint32_t size, void *tx_buffer, void *rx_buffer);

} // namespace IO
} // namespace System
