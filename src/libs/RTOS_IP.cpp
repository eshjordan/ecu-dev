#ifndef STM32_BUILD

#include "RTOS_IP.hpp"
#include "FreeRTOS_IP_Private.h"
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
extern const uint8_t ucIPAddress[4]        = {configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3};
extern const uint8_t ucNetMask[4]          = {configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3};
extern const uint8_t ucGatewayAddress[4]   = {configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2,
                                            configGATEWAY_ADDR3};
extern const uint8_t ucDNSServerAddress[4] = {configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2,
                                              configDNS_SERVER_ADDR3};
/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
extern const uint8_t ucMACAddress[6] = {configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2,
                                        configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5};

/* Set the following constant to pdTRUE to log using the method indicated by the
 * name of the constant, or pdFALSE to not log using the method indicated by the
 * name of the constant.  Options include to standard out (xLogToStdout), to a
 * disk file (xLogToFile), and to a UDP port (xLogToUDP).  If xLogToUDP is set
 * to pdTRUE then UDP messages are sent to the IP address configured as the echo
 * server address (see the configECHO_SERVER_ADDR0 definitions in
 * FreeRTOSConfig.h) and the port number set by configPRINT_PORT in
 * FreeRTOSConfig.h. */
extern const BaseType_t xLogToStdout = pdTRUE;
extern const BaseType_t xLogToFile   = pdFALSE;
extern const BaseType_t xLogToUDP    = pdFALSE;

void vLoggingPrintf(const char *pcFormatString, ...)
{
    va_list xArguments;
    char cBuffer[256];

    /* Write the string to cBuffer. */
    va_start(xArguments, pcFormatString);
    vsnprintf(cBuffer, sizeof(cBuffer), pcFormatString, xArguments);
    va_end(xArguments);

    /* Output the message. */
    printf("%s", cBuffer);
}

UBaseType_t uxRand(void)
{
    /* Seed used by the pseudo random number generator. */
    static UBaseType_t ulNextRand = 1UL;

    const uint32_t ulMultiplier = 0x015a4e35UL;
    const uint32_t ulIncrement  = 1UL;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
    return ((int)(ulNextRand >> 16UL) & 0x7fffUL);
}

extern uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort,
                                                   uint32_t ulDestinationAddress, uint16_t usDestinationPort)
{
    (void)ulSourceAddress;
    (void)usSourcePort;
    (void)ulDestinationAddress;
    (void)usDestinationPort;

    return uxRand();
}

BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber)
{
    *(pulNumber) = uxRand();
    return pdTRUE;
}

BaseType_t xApplicationDNSQueryHook(const char *pcName)
{
    BaseType_t xReturn;

    if (strcmp(pcName, "localhost") == 0)
    {
        xReturn = pdPASS;
    } else
    {
        xReturn = pdFAIL;
    }

    return xReturn;
}

#ifdef ipconfigDHCP_REGISTER_HOSTNAME

const char *pcApplicationHostnameHook(void) { return "jordan-ecu"; }

#endif /* ipconfigDHCP_REGISTER_HOSTNAME */

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* Both eNetworkUp and eNetworkDown events can be processed here. */
    if (eNetworkEvent == eNetworkUp)
    {
        /* Create the tasks that use the TCP/IP stack if they have not already
        been created. */
        if (xTasksAlreadyCreated == pdFALSE)
        {
            rtos_ip_start();

            /*
             * For convenience, tasks that use FreeRTOS+TCP can be created here
             * to ensure they are not created before the network is usable.
             */
            xTasksAlreadyCreated = pdTRUE;
        }
    }
}

void vNetworkInterfaceAllocateRAMToBuffers(
    NetworkBufferDescriptor_t pxNetworkBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS])
{
    for (int x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++)
    {
        NetworkBufferDescriptor_t *current = &pxNetworkBuffers[x];
        auto *buf                          = (uint8_t *)pvPortMalloc(ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING);
        current->pucEthernetBuffer         = buf + ipBUFFER_PADDING;

        // See
        // https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/Embedded_Ethernet_Porting.html#vNetworkInterfaceAllocateRAMToBuffers
        *((NetworkBufferDescriptor_t **)buf) = &(pxNetworkBuffers[x]);
    }
}

void ip_to_str(char dst[16], const uint32_t ip_value)
{
    uint8_t ip_written = 0;

    for (int i = 0; i < ipIP_ADDRESS_LENGTH_BYTES; i++)
    {
        ip_written += sprintf(dst + ip_written, "%u.", ip_value >> (8U * i) & 0xFFU);
    }

    dst[ip_written - 1] = '\0';
}

void print_network_stats(void)
{
    char ip[16] = {0}, netmask[16] = {0}, gateway[16] = {0}, dns[16] = {0}, mac[32] = {0};
    int ip_written = 0, netmask_written = 0, gateway_written = 0, dns_written = 0, mac_written = 0;

    const uint32_t ip_value  = FreeRTOS_GetIPAddress();
    const uint32_t net_value = FreeRTOS_GetNetmask();
    const uint32_t gw_value  = FreeRTOS_GetGatewayAddress();
    const uint32_t dns_value = FreeRTOS_GetDNSServerAddress();
    const uint8_t *mac_value = FreeRTOS_GetMACAddress();

    for (int i = 0; i < ipIP_ADDRESS_LENGTH_BYTES; i++)
    {
        ip_written += sprintf(ip + ip_written, "%u.", ip_value >> (8U * i) & 0xFFU);
        netmask_written += sprintf(netmask + netmask_written, "%u.", net_value >> (8U * i) & 0xFFU);
        gateway_written += sprintf(gateway + gateway_written, "%u.", gw_value >> (8U * i) & 0xFFU);
        dns_written += sprintf(dns + dns_written, "%u.", dns_value >> (8U * i) & 0xFFU);
        mac_written += sprintf(mac + mac_written, "%02X:", mac_value[i]);
    }

    for (int i = ipIP_ADDRESS_LENGTH_BYTES; i < ipMAC_ADDRESS_LENGTH_BYTES; i++)
    {
        mac_written += sprintf(mac + mac_written, "%02X:", mac_value[i]);
    }

    ip[ip_written - 1]           = '\0';
    netmask[netmask_written - 1] = '\0';
    gateway[gateway_written - 1] = '\0';
    dns[dns_written - 1]         = '\0';
    mac[mac_written - 1]         = '\0';

    char buffer[512];
    sprintf(buffer,
            "\nNetwork Stats:\nIP: %s\nNETMASK: %s\nGATEWAY: %s\nDNS: %s\nMAC: "
            "%s\n\n",
            ip, netmask, gateway, dns, mac);

    vLoggingPrintf(buffer);
}

void print_bind_err(BaseType_t status)
{
    switch (status)
    {
    case 0: {
        vLoggingPrintf("No bind error - successful!\n");
    }
    case -FREERTOS_EINVAL: {
        vLoggingPrintf("bind failed: The socket did not get bound, probably because the specified port number was "
                       "already in use.\n");
        break;
    }
    default: {
        vLoggingPrintf(
            "bind failed: The calling RTOS task did not get a response from the IP RTOS task to the bind request.\n");
        break;
    }
    }
}

void print_listen_err(BaseType_t status)
{
    switch (status)
    {
    case 0: {
        vLoggingPrintf("No listen error - successful!\n");
    }
    case -pdFREERTOS_ERRNO_EOPNOTSUPP: {
        vLoggingPrintf(
            "listen failed: Socket is not a valid TCP socket or socket is not in bound, but closed state!\n");
        break;
    }
    default: {
        vLoggingPrintf("listen failed: Unknown error!\n");
        break;
    }
    }
}

void print_accept_err(Socket_t status)
{
    if (status == nullptr)
    {
        vLoggingPrintf("accept failed - Connection timeout\n");
    } else if (status == FREERTOS_INVALID_SOCKET)
    {
        vLoggingPrintf("accept failed - Socket is not a valid TCP socket, or socket is not in the Listening state!\n");
    } else
    {
        vLoggingPrintf("No accept error - successful!\n");
    }
}

void print_recv_err(BaseType_t status)
{
    switch (status)
    {
    case 0: {
        vLoggingPrintf("recv failed: Timeout!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_ENOMEM: {
        vLoggingPrintf("recv failed: Not enough memory!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_ENOTCONN: {
        vLoggingPrintf("recv failed: Socket closed!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_EINTR: {
        vLoggingPrintf("recv failed: Read aborted!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_EINVAL: {
        vLoggingPrintf("recv failed: Socket is not valid, is not a TCP socket, or is not bound!\n");
        break;
    }
    default: {
        vLoggingPrintf("recv failed: Unknown error! - %d\n", status);
        break;
    }
    }
}

void print_send_err(BaseType_t status)
{
    switch (status)
    {
    case -pdFREERTOS_ERRNO_ENOTCONN: {
        vLoggingPrintf("send failed: Socket closed!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_ENOMEM: {
        vLoggingPrintf("send failed: Not enough memory!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_EINVAL: {
        vLoggingPrintf("send failed: Socket is not a valid TCP socket!\n");
        break;
    }
    case -pdFREERTOS_ERRNO_ENOSPC: {
        vLoggingPrintf("send failed: Timeout!\n");
        break;
    }
    default: {
        vLoggingPrintf("send failed: Unknown error! - %d\n", status);
        break;
    }
    }
}

#endif
