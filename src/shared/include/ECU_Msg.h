#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

#define ECU_MSG_START_BYTE 0x9DU

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT {}
#define START_BYTE_INIT =ECU_MSG_START_BYTE
#else
#define STRUCT_INIT
#define START_BYTE_INIT
#endif

// clang-format on

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

enum ECU_Command_t {
    UNKNOWN_CMD         = (uint8_t)(0U),  /* Used for error checking. */
    ECHO_CMD            = (uint8_t)(1U),  /* Echo a copy of this message exactly. */
    PING_CMD            = (uint8_t)(2U),  /* Get server to send a connection acknowledgement. */
    ACK_CMD             = (uint8_t)(3U),  /* Acknowledge a message was received correctly. */
    RESTART_CMD         = (uint8_t)(4U),  /* Restart the server. */
    STATUS_CMD          = (uint8_t)(5U),  /* Request Server status. */
    SYNC_CMD            = (uint8_t)(6U),  /* Synchronize client-server time. */
    FIRMWARE_UPDATE_CMD = (uint8_t)(7U),  /* Upgrade the ECU's Firmware, the SDK. */
    PROGRAM_UPDATE_CMD  = (uint8_t)(8U),  /* Update the ECU's programmed software, user-defined. */
    VALUE_CMD           = (uint8_t)(9U),  /* Send a raw data value. */
    PARAM_GET_CMD       = (uint8_t)(10U), /* Retreive a parameter's value. */
    PARAM_SET_CMD       = (uint8_t)(11U)  /* Set a parameter's value. */
} ALIGN;

typedef enum ECU_Command_t ECU_Command_t;

struct ECU_Msg_t {
    Header_t header STRUCT_INIT;
    char name[64] STRUCT_INIT;
    uint8_t data[8] STRUCT_INIT; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum ECU_Command_t command STRUCT_INIT;
    CRC checksum STRUCT_INIT;
} ALIGN;

typedef struct ECU_Msg_t ECU_Msg_t;

#ifdef USERSPACE_BUILD

struct ECU_Msg_t ecu_msg_make(uint32_t id, const char name[64], const void *data, ECU_Command_t command);

CRC ecu_msg_calc_checksum(ECU_Msg_t *msg);

ecu_err_t ecu_msg_check(const ECU_Msg_t *msg);

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void ecu_msg_to_str(char *str, const ECU_Msg_t *msg);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MESSAGE_H
