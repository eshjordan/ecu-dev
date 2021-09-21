#pragma once

extern "C" {

/* FreeRTOS Kernel includes */

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"

/* Prototypes for the standard FreeRTOS extern callback/hook functions we should implement (some we don't need atm). */

/**
 * @brief vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h.  It will be
 * called on each iteration of the idle task.  It is essential that code added to this hook function never attempts to
 * block in any way (for example, call xQueueReceive() with a block time specified, or call vTaskDelay()).  If
 * application tasks make use of the vTaskDelete() API function to delete themselves then it is also important that
 * vApplicationIdleHook() is permitted to return to its calling function, because it is the responsibility of the idle
 * task to clean up memory allocated by the kernel to any task that has since deleted itself.
 *
 */
void vApplicationIdleHook(void);

/**
 * @brief This function will be called by each tick interrupt if configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.
 * User code can be added here, but the tick hook is called from an interrupt context, so code must not attempt to
 * block, and only the interrupt safe FreeRTOS API functions can be used (those that end in FromISR()).
 *
 */
void vApplicationTickHook(void);

/**
 * @brief vApplicationMallocFailedHook() will only be called if configUSE_MALLOC_FAILED_HOOK is set to 1 in
 * FreeRTOSConfig.h.  It is a hook function that will get called if a call to pvPortMalloc() fails. pvPortMalloc() is
 * called internally by the kernel whenever a task, queue, timer or semaphore is created.  It is also called by various
 * parts of the demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the size of the    heap
 * available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
 * API function can be used to query the size of free heap space that remains (although it does not provide information
 * on how the remaining heap might be fragmented).  See www.freertos.org/a00111.html for more information.
 *
 */
void vApplicationMallocFailedHook(void);

/**
 * @brief Run time stack overflow checking is performed if configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This
 * hook function is called if a stack overflow is detected.  This function is provided as an example only as stack
 * overflow checking does not function when running the FreeRTOS POSIX port.
 *
 * @param pxTask
 * @param pcTaskName
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);

/**
 * @brief This function will be called once only, when the daemon task starts to execute (sometimes called the timer
 * task).  This is useful if the application includes initialisation code that would benefit from executing after the
 * scheduler has been started.
 *
 */
void vApplicationDaemonTaskStartupHook(void);

/**
 * @brief Called if an assertion passed to configASSERT() fails.  See www.freertos.org/a00110.html#configASSERT
 * for more information. Parameters are not used.
 *
 * @param pcFileName
 * @param ulLine
 */
void vAssertCalled(const char *const pcFileName, unsigned long ulLine);

/**
 * @brief configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an implementation of
 * vApplicationGetIdleTaskMemory() to provide the memory that is used by the Idle task.
 *
 * @param ppxIdleTaskTCBBuffer
 * @param ppxIdleTaskStackBuffer
 * @param pulIdleTaskStackSize
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/**
 * @brief configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the application must provide an
 * implementation of vApplicationGetTimerTaskMemory() to provide the memory that is used by the Timer service task.
 *
 * @param ppxTimerTaskTCBBuffer
 * @param ppxTimerTaskStackBuffer
 * @param pulTimerTaskStackSize
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

/* timers.h must be included after declaring the prototypes. */
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

/* Prototypes for the standard FreeRTOS+TCP extern callback/hook functions we should implement (some we don't need atm).
 */

/**
 * @brief Prototype for the function used to print out.  In this case it prints to the console before the network is
 * connected then a UDP port after the network has connected.
 *
 * @param pcFormatString Format string, standard formatting rules.
 * @param ... Format arguments.
 */
void vLoggingPrintf(const char *pcFormatString, ...);

/**
 * @brief ipconfigRAND32() is called by the IP stack to generate random numbers for things such as a DHCP transaction
 * number or initial sequence number.  Random number generation is performed via this macro to allow applications to use
 * their own random number generation method.  For example, it might be possible to generate a random number by sampling
 * noise on an analogue input.
 *
 * @return UBaseType_t
 */
UBaseType_t uxRand(void);

/**
 * @brief Callback that provides the inputs necessary to generate a randomized TCP Initial Sequence Number per RFC 6528.
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
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
 * @brief Supply a random number to FreeRTOS+TCP stack. THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM
 * NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
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

/* FreeRTOS_IP.h must be included after declaring the prototypes. */
#include "FreeRTOS_IP.h"

void vNetworkInterfaceAllocateRAMToBuffers(
    NetworkBufferDescriptor_t pxNetworkBuffers[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS]);

/**
 * @brief If ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 then FreeRTOS+TCP will call the network event hook at the
 * appropriate times. If ipconfigUSE_NETWORK_EVENT_HOOK is not set to 1 then the network event hook will never be
 * called. See: http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_UDP/API/vApplicationIPNetworkEventHook.shtml.
 *
 * @param eNetworkEvent
 */
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent);

#endif
}
