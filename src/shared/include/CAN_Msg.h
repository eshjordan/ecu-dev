#ifndef CAN_MSG_H
#define CAN_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

#define CAN_MSG_START_BYTE 0xCAU

// clang-format off

#ifdef __cplusplus
#define _CAN_MSG_HEADER_INIT {CAN_MSG_START_BYTE, 0U , 0U}
#else
#define _CAN_MSG_HEADER_INIT
#endif

// clang-format on

#define ALIGN __attribute__((aligned(4)))

#define CAN_MSG_FLAG_EXTD (1 << 31)         /* Extended Frame Format (29 bit ID) */
#define CAN_MSG_FLAG_RTR (1 << 30)          /* Message is a Remote Frame */
#define CAN_MSG_FLAG_SS (1 << 29)           /* Transmit as a Single Shot Transmission. Unused for received. */
#define CAN_MSG_FLAG_SELF (1 << 28)         /* Transmit as a Self Reception Request. Unused for received. */
#define CAN_MSG_FLAG_DLC_NON_COMP (1 << 27) /* Message's Data length code is larger than 8. */

struct CAN_Msg_t {
    Header_t header _CAN_MSG_HEADER_INIT;
    uint32_t flags _STRUCT_INIT;           /* Message flags */
    uint32_t identifier _STRUCT_INIT;      /* 11 or 29 bit identifier */
    uint8_t data_length_code _STRUCT_INIT; /* Data length code */
    uint8_t data[8] _STRUCT_INIT;          /* Data bytes (not relevant in RTR frame) */
    CRC checksum _STRUCT_INIT;             /* Message checksum. */
} ALIGN;

typedef struct CAN_Msg_t CAN_Msg_t;

#ifdef USERSPACE_BUILD

CRC can_msg_calc_checksum(CAN_Msg_t *msg);

ecu_err_t can_msg_check(const CAN_Msg_t *msg);

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void can_msg_to_str(char *str, const CAN_Msg_t *msg);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CAN_MSG_H
