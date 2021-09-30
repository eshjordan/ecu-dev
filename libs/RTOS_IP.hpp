#pragma once

extern "C" {

/* FreeRTOS Kernel includes */

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "timers.h"

#ifdef ECU_ENABLE_NETWORKING

extern const uint8_t ucMACAddress[6];
extern const uint8_t ucIPAddress[4];
extern const uint8_t ucNetMask[4];
extern const uint8_t ucGatewayAddress[4];
extern const uint8_t ucDNSServerAddress[4];

extern const BaseType_t xLogToStdout;
extern const BaseType_t xLogToFile;
extern const BaseType_t xLogToUDP;

/* Prototypes for the standard FreeRTOS+TCP extern callback/hook functions we
 * should implement (some we don't need atm).
 */

/**
 * @brief Prototype for the function used to print out.  In this case it prints
 * to the console before the network is connected then a UDP port after the
 * network has connected.
 *
 * @param pcFormatString Format string, standard formatting rules.
 * @param ... Format arguments.
 */
void vLoggingPrintf(const char *pcFormatString, ...);

/**
 * @brief ipconfigRAND32() is called by the IP stack to generate random numbers
 * for things such as a DHCP transaction number or initial sequence number.
 * Random number generation is performed via this macro to allow applications to
 * use their own random number generation method.  For example, it might be
 * possible to generate a random number by sampling noise on an analogue input.
 *
 * @return UBaseType_t
 */
UBaseType_t uxRand(void);

/**
 * @brief Callback that provides the inputs necessary to generate a randomized
 * TCP Initial Sequence Number per RFC 6528. THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 *
 * @param ulSourceAddress
 * @param usSourcePort
 * @param ulDestinationAddress
 * @param usDestinationPort
 * @return uint32_t
 */
uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort,
                                            uint32_t ulDestinationAddress, uint16_t usDestinationPort);

/**
 * @brief Supply a random number to FreeRTOS+TCP stack. THIS IS ONLY A DUMMY
 * IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE
 * IN PRODUCTION SYSTEMS.
 *
 * @param pulNumber
 * @return BaseType_t
 */
BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber);

/**
 * @brief Determine if a name lookup is for this node.
 *
 * @param pcName The name of the lookup (to check).
 * @return BaseType_t true if the name is for this node, false otherwise.
 */
BaseType_t xApplicationDNSQueryHook(const char *pcName);

/**
 * @brief Get string repr of IP address.
 *
 */
char *ip_to_str(const uint32_t ip_value);

/**
 * @brief Print network statistics.
 *
 */
void print_network_stats(void);

#ifdef ipconfigDHCP_REGISTER_HOSTNAME

/**
 * @brief Declare our hostname to a DHCP server.
 *
 * @return const char* Hostname.
 */
const char *pcApplicationHostnameHook(void);

#endif /* ipconfigDHCP_REGISTER_HOSTNAME */

/* FreeRTOS_IP.h must be included after declaring the prototypes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/**
 * @brief If ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 then FreeRTOS+TCP will
 * call the network event hook at the appropriate times. If
 * ipconfigUSE_NETWORK_EVENT_HOOK is not set to 1 then the network event hook
 * will never be called. See:
 * http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_UDP/API/vApplicationIPNetworkEventHook.shtml.
 *
 * @param eNetworkEvent
 */
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent);

#endif
}
