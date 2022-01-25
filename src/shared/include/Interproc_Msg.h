#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT {}
#else
#define STRUCT_INIT
#endif

// clang-format on

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

struct Interproc_Msg_t {
    enum Interproc_Command_t {
        UNKNOWN_CMD         = (u8)(0U),  /* Used for error checking. */
        ECHO_CMD            = (u8)(1U),  /* Echo a copy of this message exactly. */
        PING_CMD            = (u8)(2U),  /* Get server to send a connection acknowledgement. */
        ACK_CMD             = (u8)(3U),  /* Acknowledge a message was received correctly. */
        RESTART_CMD         = (u8)(4U),  /* Restart the server. */
        STATUS_CMD          = (u8)(5U),  /* Request Server status. */
        SYNC_CMD            = (u8)(6U),  /* Synchronize client-server time. */
        FIRMWARE_UPDATE_CMD = (u8)(7U),  /* Upgrade the ECU's Firmware, the SDK. */
        PROGRAM_UPDATE_CMD  = (u8)(8U),  /* Update the ECU's programmed software, user-defined. */
        VALUE_CMD           = (u8)(9U),  /* Send a raw data value. */
        PARAM_GET_CMD       = (u8)(10U), /* Retreive a parameter's value. */
        PARAM_SET_CMD       = (u8)(11U)  /* Set a parameter's value. */
    } ALIGN;

    Header_t header STRUCT_INIT;
    char name[64] STRUCT_INIT;
    u8 data[8] STRUCT_INIT; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum Interproc_Command_t command STRUCT_INIT;
    CRC checksum STRUCT_INIT;
} ALIGN;

#ifdef __cplusplus
using Interproc_Msg_t = Interproc_Msg_t::Interproc_Command_t;
#else
typedef enum Interproc_Command_t Interproc_Command_t;
#endif

typedef struct Interproc_Msg_t Interproc_Msg_t;


#ifdef USERSPACE_BUILD

struct Interproc_Msg_t interproc_msg_make(u32 id, const char name[64], const void *data,
                                          Interproc_Command_t command);

CRC interproc_msg_calc_checksum(Interproc_Msg_t *msg);

ecu_err_t interproc_msg_check(const Interproc_Msg_t *msg);

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void interproc_msg_to_str(char *str, const Interproc_Msg_t *msg);

#endif // USERSPACE_BUILD

#ifdef __cplusplus
} // extern "C"
#endif


#endif // MESSAGE_H
