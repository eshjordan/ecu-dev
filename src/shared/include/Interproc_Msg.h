#ifndef INTERPROC_MESSAGE_H
#define INTERPROC_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

#define INTERPROC_MSG_START_BYTE 0xABU

// clang-format off

#ifdef __cplusplus
#define _INTERPROC_MSG_HEADER_INIT ={INTERPROC_MSG_START_BYTE, 0U, 0U}
#else
#define _INTERPROC_MSG_HEADER_INIT
#endif

// clang-format on


enum Interproc_Command_t {
	CMD_UNKNOWN         = (uint8_t)(0U),    /* Used for error checking. */
	CMD_ECHO            = (uint8_t)(1U),    /* Echo a copy of this message exactly. */
	CMD_PING            = (uint8_t)(2U),    /* Get server to send a connection acknowledgement. */
	CMD_ACK             = (uint8_t)(3U),    /* Acknowledge a message was received correctly. */
	CMD_RESET           = (uint8_t)(4U),    /* Restart the server. */
	CMD_STATUS          = (uint8_t)(5U),    /* Request Server status. */
	CMD_SYNC            = (uint8_t)(6U),    /* Synchronize client-server time. */
	CMD_FIRMWARE_UPDATE = (uint8_t)(7U),    /* Upgrade the ECU's Firmware, the SDK. */
	CMD_PROGRAM_UPDATE  = (uint8_t)(8U),    /* Update the ECU's programmed software, user-defined. */
	CMD_VALUE           = (uint8_t)(9U),    /* Send a raw data value. */
	CMD_PARAM_GET       = (uint8_t)(10U),   /* Retreive a parameter's value. */
	CMD_PARAM_SET       = (uint8_t)(11U),   /* Set a parameter's value. */
	CMD_CHANNEL_GET     = (uint8_t)(12U)    /* Retreive a channel's value. */
} ALIGN;

typedef enum Interproc_Command_t Interproc_Command_t;

struct Interproc_Msg_t {
	Header_t header _INTERPROC_MSG_HEADER_INIT;
	Interproc_Command_t command : 8;
	uint8_t _pad1 _STRUCT_INIT;
	uint8_t _pad2 _STRUCT_INIT;
	uint8_t _pad3 _STRUCT_INIT;
	uint8_t data[64] _STRUCT_INIT;
	CRC checksum _STRUCT_INIT;
} ALIGN;

typedef struct Interproc_Msg_t Interproc_Msg_t;

#ifdef USERSPACE_BUILD

struct Interproc_Msg_t interproc_msg_make(Interproc_Command_t command,
					  const void *const data, uint16_t length);

CRC interproc_msg_calc_checksum(Interproc_Msg_t *const msg);

ecu_err_t interproc_msg_check(const Interproc_Msg_t *const msg);

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void interproc_msg_to_str(char *str, const Interproc_Msg_t *const msg);

#endif // USERSPACE_BUILD

#ifdef __cplusplus
} // extern "C"
#endif

#endif // INTERPROC_MESSAGE_H
