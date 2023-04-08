#include "Error.hpp"
#include "main.h"
#include <stdarg.h>
#include <stdio.h>


void ecu_fatal_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    Error_Handler();
}
