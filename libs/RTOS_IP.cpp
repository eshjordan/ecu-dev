#include "RTOS_IP.hpp"
#include <cstdarg>
#include <cstdio>

#ifdef ECU_ENABLE_NETWORKING

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
extern const uint8_t ucIPAddress[4] = {configIP_ADDR0, configIP_ADDR1,
                                       configIP_ADDR2, configIP_ADDR3};
extern const uint8_t ucNetMask[4] = {configNET_MASK0, configNET_MASK1,
                                     configNET_MASK2, configNET_MASK3};
extern const uint8_t ucGatewayAddress[4] = {
    configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2,
    configGATEWAY_ADDR3};
extern const uint8_t ucDNSServerAddress[4] = {
    configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3};
/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
extern const uint8_t ucMACAddress[6] = {configMAC_ADDR0, configMAC_ADDR1,
                                        configMAC_ADDR2, configMAC_ADDR3,
                                        configMAC_ADDR4, configMAC_ADDR5};

/* Set the following constant to pdTRUE to log using the method indicated by the
 * name of the constant, or pdFALSE to not log using the method indicated by the
 * name of the constant.  Options include to standard out (xLogToStdout), to a
 * disk file (xLogToFile), and to a UDP port (xLogToUDP).  If xLogToUDP is set
 * to pdTRUE then UDP messages are sent to the IP address configured as the echo
 * server address (see the configECHO_SERVER_ADDR0 definitions in
 * FreeRTOSConfig.h) and the port number set by configPRINT_PORT in
 * FreeRTOSConfig.h. */
extern const BaseType_t xLogToStdout = pdTRUE;
extern const BaseType_t xLogToFile = pdFALSE;
extern const BaseType_t xLogToUDP = pdFALSE;

void vLoggingPrintf(const char *pcFormatString, ...) {
  va_list xArguments;
  char cBuffer[256];

  /* Write the string to cBuffer. */
  va_start(xArguments, pcFormatString);
  vsnprintf(cBuffer, sizeof(cBuffer), pcFormatString, xArguments);
  va_end(xArguments);

  /* Output the message. */
  printf("%s", cBuffer);
}

UBaseType_t uxRand(void) {
  /* Seed used by the pseudo random number generator. */
  static UBaseType_t ulNextRand = 1UL;

  const uint32_t ulMultiplier = 0x015a4e35UL;
  const uint32_t ulIncrement = 1UL;

  /* Utility function to generate a pseudo random number. */

  ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
  return ((int)(ulNextRand >> 16UL) & 0x7fffUL);
}

extern uint32_t ulApplicationGetNextSequenceNumber(
    uint32_t ulSourceAddress, uint16_t usSourcePort,
    uint32_t ulDestinationAddress, uint16_t usDestinationPort) {
  (void)ulSourceAddress;
  (void)usSourcePort;
  (void)ulDestinationAddress;
  (void)usDestinationPort;

  return uxRand();
}

BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber) {
  *(pulNumber) = uxRand();
  return pdTRUE;
}

BaseType_t xApplicationDNSQueryHook(const char *pcName) {
  BaseType_t xReturn;

  if (strcmp(pcName, "localhost") == 0) {
    xReturn = pdPASS;
  } else {
    xReturn = pdFAIL;
  }

  return xReturn;
}

#ifdef ipconfigDHCP_REGISTER_HOSTNAME

const char *pcApplicationHostnameHook(void) { return "jordan-ecu"; }

#endif /* ipconfigDHCP_REGISTER_HOSTNAME */

/* Forward declare the Server::start() function. */
namespace System {
namespace Impl {
class Server {
public:
  static void start();
};
} // namespace Impl
} // namespace System

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent) {
  static BaseType_t xTasksAlreadyCreated = pdFALSE;

  /* Both eNetworkUp and eNetworkDown events can be processed here. */
  if (eNetworkEvent == eNetworkUp) {
    /* Create the tasks that use the TCP/IP stack if they have not already
    been created. */
    if (xTasksAlreadyCreated == pdFALSE) {
      System::Impl::Server::start();

      /*
       * For convenience, tasks that use FreeRTOS+TCP can be created here
       * to ensure they are not created before the network is usable.
       */
      xTasksAlreadyCreated = pdTRUE;
    }
  }
}

char *ip_to_str(const uint32_t ip_value) {
  uint8_t ip_written = 0;
  char *ip = (char *)pvPortMalloc(sizeof(char) * 16);

  for (int i = 0; i < ipIP_ADDRESS_LENGTH_BYTES; i++) {
    ip_written += sprintf(ip + ip_written, "%u.", ip_value >> (8U * i) & 0xFFU);
  }

  ip[ip_written - 1] = '\0';

  return ip;
}

void print_network_stats(void) {
  char ip[16] = {0}, netmask[16] = {0}, gateway[16] = {0}, dns[16] = {0},
       mac[32] = {0};
  int ip_written = 0, netmask_written = 0, gateway_written = 0, dns_written = 0,
      mac_written = 0;

  const uint32_t ip_value = FreeRTOS_GetIPAddress();
  const uint32_t net_value = FreeRTOS_GetNetmask();
  const uint32_t gw_value = FreeRTOS_GetGatewayAddress();
  const uint32_t dns_value = FreeRTOS_GetDNSServerAddress();
  const uint8_t *mac_value = FreeRTOS_GetMACAddress();

  for (int i = 0; i < ipIP_ADDRESS_LENGTH_BYTES; i++) {
    ip_written += sprintf(ip + ip_written, "%u.", ip_value >> (8U * i) & 0xFFU);
    netmask_written += sprintf(netmask + netmask_written, "%u.",
                               net_value >> (8U * i) & 0xFFU);
    gateway_written +=
        sprintf(gateway + gateway_written, "%u.", gw_value >> (8U * i) & 0xFFU);
    dns_written +=
        sprintf(dns + dns_written, "%u.", dns_value >> (8U * i) & 0xFFU);
    mac_written += sprintf(mac + mac_written, "%02X:", mac_value[i]);
  }

  for (int i = ipIP_ADDRESS_LENGTH_BYTES; i < ipMAC_ADDRESS_LENGTH_BYTES; i++) {
    mac_written += sprintf(mac + mac_written, "%02X:", mac_value[i]);
  }

  ip[ip_written - 1] = '\0';
  netmask[netmask_written - 1] = '\0';
  gateway[gateway_written - 1] = '\0';
  dns[dns_written - 1] = '\0';
  mac[mac_written - 1] = '\0';

  char buffer[512];
  sprintf(buffer,
          "\nNetwork Stats:\nIP: %s\nNETMASK: %s\nGATEWAY: %s\nDNS: %s\nMAC: "
          "%s\n\n",
          ip, netmask, gateway, dns, mac);

  vLoggingPrintf(buffer);
}

#endif
