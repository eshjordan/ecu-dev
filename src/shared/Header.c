#include "Header.h"

Header_t header_make(uint8_t start_byte, uint16_t length)
{
    Header_t hdr = {.start_byte = (start_byte), ._pad1 = 0U, .length = (length)};
    return hdr;
}
