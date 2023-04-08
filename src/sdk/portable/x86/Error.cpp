#include "Error.hpp"
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>

void ecu_fatal_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    char output[1024] = {0};
    vsprintf(output, message, args);
    va_end(args);
    throw std::runtime_error(output);
}
