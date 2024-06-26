#pragma once

#include <stdio.h>

extern "C" {

/* FreeRTOS Kernel includes */

#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"
#include "task.h"

#ifndef STM32_BUILD

// extern void *memcpy(void *__restrict __dest, const void *__restrict __src, size_t __n) __THROW __nonnull((1, 2));

/* Prototypes for the standard FreeRTOS extern callback/hook functions we should
 * implement (some we don't need atm). */

/**
 * @brief vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is
 * set to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the
 * idle task.  It is essential that code added to this hook function never
 * attempts to block in any way (for example, call xQueueReceive() with a block
 * time specified, or call vTaskDelay()).  If application tasks make use of the
 * vTaskDelete() API function to delete themselves then it is also important
 * that vApplicationIdleHook() is permitted to return to its calling function,
 * because it is the responsibility of the idle task to clean up memory
 * allocated by the kernel to any task that has since deleted itself.
 *
 */
void vApplicationIdleHook(void);

/**
 * @brief This function will be called by each tick interrupt if
 * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be added
 * here, but the tick hook is called from an interrupt context, so code must not
 * attempt to block, and only the interrupt safe FreeRTOS API functions can be
 * used (those that end in FromISR()).
 *
 */
void vApplicationTickHook(void);

/**
 * @brief vApplicationMallocFailedHook() will only be called if
 * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
 * function that will get called if a call to pvPortMalloc() fails.
 * pvPortMalloc() is called internally by the kernel whenever a task, queue,
 * timer or semaphore is created.  It is also called by various parts of the
 * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
 * size of the    heap available to pvPortMalloc() is defined by
 * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API
 * function can be used to query the size of free heap space that remains
 * (although it does not provide information on how the remaining heap might be
 * fragmented).  See www.freertos.org/a00111.html for more information.
 *
 */
void vApplicationMallocFailedHook(void);

/**
 * @brief Run time stack overflow checking is performed if
 * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook function is
 * called if a stack overflow is detected.  This function is provided as an
 * example only as stack overflow checking does not function when running the
 * FreeRTOS POSIX port.
 *
 * @param pxTask
 * @param pcTaskName
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);

/**
 * @brief This function will be called once only, when the daemon task starts to
 * execute (sometimes called the timer task).  This is useful if the application
 * includes initialisation code that would benefit from executing after the
 * scheduler has been started.
 *
 */
void vApplicationDaemonTaskStartupHook(void);

/**
 * @brief Called if an assertion passed to configASSERT() fails.  See
 * www.freertos.org/a00110.html#configASSERT for more information. Parameters
 * are not used.
 *
 * @param pcFileName
 * @param ulLine
 */
void vAssertCalled(const char *const pcFileName, unsigned long ulLine);

/**
 * @brief configUSE_STATIC_ALLOCATION is set to 1, so the application must
 * provide an implementation of vApplicationGetIdleTaskMemory() to provide the
 * memory that is used by the Idle task.
 *
 * @param ppxIdleTaskTCBBuffer
 * @param ppxIdleTaskStackBuffer
 * @param pulIdleTaskStackSize
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/**
 * @brief configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so
 * the application must provide an implementation of
 * vApplicationGetTimerTaskMemory() to provide the memory that is used by the
 * Timer service task.
 *
 * @param ppxTimerTaskTCBBuffer
 * @param ppxTimerTaskStackBuffer
 * @param pulTimerTaskStackSize
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

#endif

/* timers.h must be included after declaring the prototypes. */
#include "timers.h"

#include "projdefs.h"
}
