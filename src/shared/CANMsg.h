#ifndef CANMSG_H
#define CANMSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CAN_MSG_FLAG_EXTD (1 << 31)         /* Extended Frame Format (29 bit ID) */
#define CAN_MSG_FLAG_RTR (1 << 30)          /* Message is a Remote Frame */
#define CAN_MSG_FLAG_SS (1 << 29)           /* Transmit as a Single Shot Transmission. Unused for received. */
#define CAN_MSG_FLAG_SELF (1 << 28)         /* Transmit as a Self Reception Request. Unused for received. */
#define CAN_MSG_FLAG_DLC_NON_COMP (1 << 27) /* Message's Data length code is larger than 8. */

struct CANMsg_t {
    uint32_t flags;           /* Message flags */
    uint32_t identifier;      /* 11 or 29 bit identifier */
    uint8_t data_length_code; /* Data length code */
    uint8_t data[8];          /* Data bytes (not relevant in RTR frame) */
};

typedef struct CANMsg_t CANMsg_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CANMSG_H
