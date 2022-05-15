#include "CAN_Msg.h"


int port_init_io(void);

uint32_t read_analogue_input(int channel);

uint32_t read_hall_input(int channel);

uint8_t read_digital_input(int channel);

void write_analogue_output(int channel, uint32_t value);

void write_digital_output(int channel, uint8_t value);

void write_pwm_output(int channel, uint16_t duty, uint32_t freq, uint8_t duty_resolution);

CAN_Msg_t read_can_input(int bus, int id);

void write_can_output(int bus, int id, CAN_Msg_t msg);

int spi_read(int channel, uint32_t size, void *buffer);

int spi_write(int channel, uint32_t size, void *buffer);

int spi_transfer(int channel, uint32_t size, void *tx_buffer, void *rx_buffer);
