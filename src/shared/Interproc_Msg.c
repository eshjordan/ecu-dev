#include "Interproc_Msg.h"

#ifdef USERSPACE_BUILD

#include <stddef.h>
#include <string.h>

Interproc_Msg_t interproc_msg_make(Interproc_Command_t command,
				   const void *const data, uint16_t length)
{
	Interproc_Msg_t dst = {
		.header = header_make(INTERPROC_MSG_START_BYTE, length),
		.command = command,
		.data = {0}
	};

	if (data) {
		memcpy(dst.data, data, length);
	}

	interproc_msg_calc_checksum(&dst);

	return dst;
}

CRC interproc_msg_calc_checksum(Interproc_Msg_t *const msg)
{
	return msg->checksum =
		       calc_crc(msg, offsetof(Interproc_Msg_t, checksum));
}

ecu_err_t interproc_msg_check(const Interproc_Msg_t *const msg)
{
	if (msg->checksum !=
	    calc_crc(msg, offsetof(Interproc_Msg_t, checksum))) {
		return -ERR_CRC_FAILED;
	}
	return ERR_OK;
}

#endif
