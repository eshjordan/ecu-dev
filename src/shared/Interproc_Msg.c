#include "Interproc_Msg.h"

#ifdef USERSPACE_BUILD

#include <stddef.h>
#include <string.h>

Interproc_Msg_t interproc_msg_make(Interproc_Command_t command,
				   const void *const data)
{
	Interproc_Msg_t dst = { .command = command };

	if (data) {
		memcpy(dst.data, data, sizeof(dst.data));
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
	return 1;
}

#endif
