#include "ECU_Error.h"

#ifdef USERSPACE_BUILD

#include <stdio.h>

void ecu_err_to_str(char *str, const ecu_err_t err_code)
{
    switch (err_code)
    {
    case -ERR_CRC_FAILED: {
        sprintf(str, "CRC Failed!");
        return;
    }
    case -ERR_INVALID_LENGTH: {
        sprintf(str, "Invalid Length!");
        return;
    }
    case -ERR_INVALID_START_BYTE: {
        sprintf(str, "Invalid Start Byte!");
        return;
    }
    default: {
        sprintf(str, "Unknown Error!");
        return;
    }
    }
}

#endif
