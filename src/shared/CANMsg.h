#ifndef CANMSG_H
#define CANMSG_H

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

#define ALIGN __attribute__((aligned(4)))

#define CAN_MSG_FLAG_EXTD (1 << 31)         /* Extended Frame Format (29 bit ID) */
#define CAN_MSG_FLAG_RTR (1 << 30)          /* Message is a Remote Frame */
#define CAN_MSG_FLAG_SS (1 << 29)           /* Transmit as a Single Shot Transmission. Unused for received. */
#define CAN_MSG_FLAG_SELF (1 << 28)         /* Transmit as a Self Reception Request. Unused for received. */
#define CAN_MSG_FLAG_DLC_NON_COMP (1 << 27) /* Message's Data length code is larger than 8. */

struct CANMsg_t {
    Header_t header STRUCT_INIT;
    uint32_t flags STRUCT_INIT;           /* Message flags */
    uint32_t identifier STRUCT_INIT;      /* 11 or 29 bit identifier */
    uint8_t data_length_code STRUCT_INIT; /* Data length code */
    uint8_t data[8] STRUCT_INIT;          /* Data bytes (not relevant in RTR frame) */
    CRC checksum STRUCT_INIT;             /* Message checksum. */
} ALIGN;

typedef struct CANMsg_t CANMsg_t;

CRC calc_can_checksum(CANMsg_t *msg);

int check_can(const CANMsg_t *msg);

void can_err_to_str(char *str, int err_code);

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void CANMsg_to_str(char *str, const CANMsg_t *msg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CANMSG_H
