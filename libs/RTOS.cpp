#include "RTOS.hpp"
// #include "../src/Server.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unistd.h>

/**
 * @brief When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can use a callback function to
 * optionally provide the memory required by the idle and timer tasks.  This is the stack that will be used by the timer
 * task.  It is declared here, as a global, so it can be checked by a test that is implemented in a different file.
 *
 */
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

/**
 * @brief Override the new operator to use the FreeRTOS malloc function.
 *
 * @param size Bytes to allocate.
 * @return void* Pointer to the allocated memory.
 */
void *operator new(size_t size) { return pvPortMalloc(size); }

/**
 * @brief Override the new operator to use the FreeRTOS malloc function.
 *
 * @param size Bytes to allocate.
 * @return void* Pointer to the allocated memory.
 */
void *operator new[](size_t size) { return pvPortMalloc(size); }

/**
 * @brief Override the delete operator to use the FreeRTOS free function.
 *
 * @param ptr Pointer to the memory to free.
 */
void operator delete(void *ptr) noexcept { vPortFree(ptr); }

/**
 * @brief Override the delete operator to use the FreeRTOS free function.
 *
 * @param ptr Pointer to the memory to free.
 */
void operator delete[](void *ptr) noexcept { vPortFree(ptr); }

void vApplicationIdleHook(void) { usleep(15000); }

void vApplicationMallocFailedHook(void) { vAssertCalled(__FILE__, __LINE__); }

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    vAssertCalled(__FILE__, __LINE__);
}

void vApplicationDaemonTaskStartupHook(void) {}

void vApplicationTickHook(void) {}

void vAssertCalled(const char *const pcFileName, unsigned long ulLine)
{
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    (void)ulLine;
    (void)pcFileName;

    taskENTER_CRITICAL();
    {
        char str[256];
#ifdef ECU_ENABLE_NETWORKING
        vLoggingPrintf("Assertion failed in %s at line %lu\n", pcFileName, ulLine);
#else
        sprintf(str, "Assertion failed in %s at line %lu\n", pcFileName, ulLine);
        throw std::runtime_error(str);
#endif

        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while (ulSetToNonZeroInDebuggerToContinue == (uint32_t)0)
        {
            __asm volatile("NOP");
            __asm volatile("NOP");
        }
    }
    taskEXIT_CRITICAL();
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be
     * allocated on the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be
     * allocated on the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#ifdef ECU_ENABLE_NETWORKING

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
 * name of the constant.  Options include to standard out (xLogToStdout), to a disk
 * file (xLogToFile), and to a UDP port (xLogToUDP).  If xLogToUDP is set to pdTRUE
 * then UDP messages are sent to the IP address configured as the echo server
 * address (see the configECHO_SERVER_ADDR0 definitions in FreeRTOSConfig.h) and
 * the port number set by configPRINT_PORT in FreeRTOSConfig.h. */
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

/* Forward declare the Server::start() function. */
namespace System {
namespace Impl {
class Server
{
public:
    static void start();
};
} // namespace Impl
} // namespace System

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
            System::Impl::Server::start();

            /*
             * For convenience, tasks that use FreeRTOS+TCP can be created here
             * to ensure they are not created before the network is usable.
             */
            xTasksAlreadyCreated = pdTRUE;
        }
    }
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
    sprintf(buffer, "\nNetwork Stats:\nIP: %s\nNETMASK: %s\nGATEWAY: %s\nDNS: %s\nMAC: %s\n\n", ip, netmask, gateway,
            dns, mac);

    vLoggingPrintf(buffer);
}

#endif
