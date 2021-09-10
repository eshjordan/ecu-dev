#include "application.h"

/*-----------------------------------------------------------*/

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It
 * is declared here, as a global, so it can be checked by a test that is
 * implemented in a different file. */
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

/* Notes if the trace is running or not. */
#if (projCOVERAGE_TEST == 1)
static BaseType_t xTraceRunning = pdFALSE;
#else
static BaseType_t xTraceRunning = pdTRUE;
#endif
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then
     * the size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might
     * be fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled(__FILE__, __LINE__);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */

    usleep(15000);
    traceOnEnter();

#if (mainSELECTED_APPLICATION == FULL_DEMO)
    {
        /* Call the idle task processing used by the full demo.  The simple
         * blinky demo does not use the idle task hook. */
        vFullDemoIdleFunction();
    }
#endif
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
     * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     * function is called if a stack overflow is detected.  This function is
     * provided as an example only as stack overflow checking does not function
     * when running the FreeRTOS POSIX port. */
    vAssertCalled(__FILE__, __LINE__);
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
    /* This function will be called by each tick interrupt if
     * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
     * added here, but the tick hook is called from an interrupt context, so
     * code must not attempt to block, and only the interrupt safe FreeRTOS API
     * functions can be used (those that end in FromISR()). */

#if (mainSELECTED_APPLICATION == FULL_DEMO)
    {
        vFullDemoTickHookFunction();
    }
#endif /* mainSELECTED_APPLICATION */
}

void traceOnEnter()
{
#if (TRACE_ON_ENTER == 1)
    int xReturn;
    struct timeval tv = {0L, 0L};
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    xReturn = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

    if (xReturn > 0)
    {
        if (xTraceRunning == pdTRUE) { prvSaveTraceFile(); }

        /* clear the buffer */
        char buffer[0];
        read(STDIN_FILENO, &buffer, 1);
    }
#endif /* if ( TRACE_ON_ENTER == 1 ) */
}

void vLoggingPrintf(const char *pcFormat, ...)
{
    va_list arg;

    va_start(arg, pcFormat);
    vprintf(pcFormat, arg);
    va_end(arg);
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook(void)
{
    /* This function will be called once only, when the daemon task starts to
     * execute    (sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled(const char *const pcFileName, unsigned long ulLine)
{
    static BaseType_t xPrinted                           = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
     * http://www.freertos.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    (void)ulLine;
    (void)pcFileName;

    taskENTER_CRITICAL();
    {
        /* Stop the trace recording. */
        if (xPrinted == pdFALSE)
        {
            xPrinted = pdTRUE;

            if (xTraceRunning == pdTRUE) { prvSaveTraceFile(); }
        }

        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while (ulSetToNonZeroInDebuggerToContinue == 0)
        {
            __asm volatile("NOP");
            __asm volatile("NOP");
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

static void prvSaveTraceFile(void)
{
/* Tracing is not used when code coverage analysis is being performed. */
#if (projCOVERAGE_TEST != 1)
    {
        FILE *pxOutputFile;

        vTraceStop();

        pxOutputFile = fopen("Trace.dump", "wb");

        if (pxOutputFile != NULL)
        {
            // fwrite(RecorderDataPtr, sizeof(RecorderDataType), 1, pxOutputFile);
            fclose(pxOutputFile);
            printf("\r\nTrace output saved to Trace.dump\r\n");
        } else
        {
            printf("\r\nFailed to create trace dump file\r\n");
        }
    }
#endif /* if ( projCOVERAGE_TEST != 1 ) */
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
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
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of
 * vApplicationGetTimerTaskMemory() to provide the memory that is used by the
 * Timer service task. */
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

/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 *******************************************************************************
 * NOTE 1: The POSIX port is a simulation (or is that emulation?) only!  Do not
 * expect to get real time behaviour from the POSIX port or this demo
 * application.  It is provided as a convenient development and demonstration
 * test bed only.
 *
 * Linux will not be running the FreeRTOS simulator threads continuously, so
 * the timing information in the FreeRTOS+Trace logs have no meaningful units.
 * See the documentation page for the Linux simulator for an explanation of
 * the slow timing:
 * https://www.freertos.org/FreeRTOS-simulator-for-Linux.html
 * - READ THE WEB DOCUMENTATION FOR THIS PORT FOR MORE INFORMATION ON USING IT -
 *
 * NOTE 2:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the comprehensive test and demo version.
 *
 * NOTE 3:  This file only contains the source code that is specific to the
 * full demo.  Generic functions, such FreeRTOS hook functions, are defined in
 * main.c.
 *******************************************************************************
 *
 * main() creates all the demo application tasks, then starts the scheduler.
 * The web documentation provides more details of the standard demo application
 * tasks, which provide no particular functionality but do provide a good
 * example of how to use the FreeRTOS API.
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Check" task - This only executes every five seconds but has a high priority
 * to ensure it gets processor time.  Its main function is to check that all the
 * standard demo tasks are still operational.  While no errors have been
 * discovered the check task will print out "OK" and the current simulated tick
 * time.  If an error is discovered in the execution of a task then the check
 * task will print out an appropriate error message.
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

/* Standard demo includes. */
#include "AbortDelay.h"
#include "BlockQ.h"
#include "EventGroupsDemo.h"
#include "GenQTest.h"
#include "IntSemTest.h"
#include "MessageBufferAMP.h"
#include "MessageBufferDemo.h"
#include "PollQ.h"
#include "QPeek.h"
#include "QueueOverwrite.h"
#include "QueueSet.h"
#include "QueueSetPolling.h"
#include "StaticAllocation.h"
#include "StreamBufferDemo.h"
#include "StreamBufferInterrupt.h"
#include "TaskNotify.h"
#include "TimerDemo.h"
#include "blocktim.h"
#include "console.h"
#include "countsem.h"
#include "death.h"
#include "dynamic.h"
#include "flop.h"
#include "integer.h"
#include "recmutex.h"
#include "semtest.h"

/* Priorities at which the tasks are created. */
#define mainCHECK_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define mainQUEUE_POLL_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainSEM_TEST_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainBLOCK_Q_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainCREATOR_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainFLASH_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainINTEGER_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainGEN_QUEUE_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainFLOP_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainQUEUE_OVERWRITE_PRIORITY (tskIDLE_PRIORITY)

#define mainTIMER_TEST_PERIOD (50)

/*
 * Exercises code that is not otherwise covered by the standard demo/test
 * tasks.
 */
extern BaseType_t xRunCodeCoverageTestAdditions(void);

/* Task function prototypes. */
static void prvCheckTask(void *pvParameters);

/* A task that is created from the idle task to test the functionality of
 * eTaskStateGet(). */
static void prvTestTask(void *pvParameters);

/*
 * Called from the idle task hook function to demonstrate a few utility
 * functions that are not demonstrated by any of the standard demo tasks.
 */
static void prvDemonstrateTaskStateAndHandleGetFunctions(void);

/*
 * Called from the idle task hook function to demonstrate the use of
 * xTimerPendFunctionCall() as xTimerPendFunctionCall() is not demonstrated by
 * any of the standard demo tasks.
 */
static void prvDemonstratePendingFunctionCall(void);

/*
 * The function that is pended by prvDemonstratePendingFunctionCall().
 */
static void prvPendedFunction(void *pvParameter1, uint32_t ulParameter2);

/*
 * prvDemonstrateTimerQueryFunctions() is called from the idle task hook
 * function to demonstrate the use of functions that query information about a
 * software timer.  prvTestTimerCallback() is the callback function for the
 * timer being queried.
 */
static void prvDemonstrateTimerQueryFunctions(void);
static void prvTestTimerCallback(TimerHandle_t xTimer);

/*
 * A task to demonstrate the use of the xQueueSpacesAvailable() function.
 */
static void prvDemoQueueSpaceFunctions(void *pvParameters);

/*
 * Tasks that ensure indefinite delays are truly indefinite.
 */
static void prvPermanentlyBlockingSemaphoreTask(void *pvParameters);
static void prvPermanentlyBlockingNotificationTask(void *pvParameters);

/*
 * The test function and callback function used when exercising the timer AP
 * function that changes the timer's auto-reload mode.
 */
static void prvDemonstrateChangingTimerReloadMode(void *pvParameters);
static void prvReloadModeTestTimerCallback(TimerHandle_t xTimer);

/*-----------------------------------------------------------*/

/* The variable into which error messages are latched. */
static char *pcStatusMessage = "OK: No errors";
int xErrorCount              = 0;

/* This semaphore is created purely to test using the vSemaphoreDelete() and
 * semaphore tracing API functions.  It has no other purpose. */
static SemaphoreHandle_t xMutexToDelete = NULL;

/*-----------------------------------------------------------*/

static void prvCheckTask(void *pvParameters)
{
    TickType_t xNextWakeTime;
    const TickType_t xCycleFrequency = pdMS_TO_TICKS(10000UL);
    HeapStats_t xHeapStats;

    /* Just to remove compiler warning. */
    (void)pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil(&xNextWakeTime, xCycleFrequency);

/* Check the standard demo tasks are running without error. */
#if (configUSE_PREEMPTION != 0)
        {
            /* These tasks are only created when preemption is used. */
            if (xAreTimerDemoTasksStillRunning(xCycleFrequency) != pdTRUE)
            {
                pcStatusMessage = "Error: TimerDemo";
                xErrorCount++;
            }
        }
#endif

        if (xAreStreamBufferTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error:  StreamBuffer";
            xErrorCount++;
        } else if (xAreMessageBufferTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error:  MessageBuffer";
            xErrorCount++;
        } else if (xAreTaskNotificationTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error:  Notification";
            xErrorCount++;
        }

        /* else if( xAreTaskNotificationArrayTasksStillRunning() != pdTRUE )
         * {
         * pcStatusMessage = "Error:  NotificationArray";
         *  xErrorCount++;
         * } */
        else if (xAreInterruptSemaphoreTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: IntSem";
            xErrorCount++;
        } else if (xAreEventGroupTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: EventGroup";
            xErrorCount++;
        } else if (xAreIntegerMathsTaskStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: IntMath";
            xErrorCount++;
        } else if (xAreGenericQueueTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: GenQueue";
            xErrorCount++;
        } else if (xAreQueuePeekTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: QueuePeek";
            xErrorCount++;
        } else if (xAreBlockingQueuesStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: BlockQueue";
            xErrorCount++;
        } else if (xAreSemaphoreTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: SemTest";
            xErrorCount++;
        } else if (xArePollingQueuesStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: PollQueue";
            xErrorCount++;
        } else if (xAreMathsTaskStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Flop";
            xErrorCount++;
        } else if (xAreRecursiveMutexTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: RecMutex";
            xErrorCount++;
        } else if (xAreCountingSemaphoreTasksStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: CountSem";
            xErrorCount++;
        } else if (xIsCreateTaskStillRunning() != pdTRUE)
        {
            pcStatusMessage = "Error: Death";
            xErrorCount++;
        } else if (xAreDynamicPriorityTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Dynamic";
            xErrorCount++;
        } else if (xIsQueueOverwriteTaskStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Queue overwrite";
            xErrorCount++;
        } else if (xAreBlockTimeTestTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Block time";
            xErrorCount++;
        } else if (xAreAbortDelayTestTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Abort delay";
            xErrorCount++;
        } else if (xIsInterruptStreamBufferDemoStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Stream buffer interrupt";
            xErrorCount++;
        } else if (xAreMessageBufferAMPTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Message buffer AMP";
            xErrorCount++;
        }

#if (configUSE_QUEUE_SETS == 1)
        else if (xAreQueueSetTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Queue set";
            xErrorCount++;
        } else if (xAreQueueSetPollTasksStillRunning() != pdPASS)
        {
            pcStatusMessage = "Error: Queue set polling";
            xErrorCount++;
        }
#endif /* if ( configUSE_QUEUE_SETS == 1 ) */

#if (configSUPPORT_STATIC_ALLOCATION == 1)
        else if (xAreStaticAllocationTasksStillRunning() != pdPASS)
        {
            xErrorCount++;
            pcStatusMessage = "Error: Static allocation";
        }
#endif /* configSUPPORT_STATIC_ALLOCATION */

        printf("%s - tick count %lu \r\n", pcStatusMessage, xTaskGetTickCount());

        if (xErrorCount != 0) { exit(1); }

        /* Reset the error condition */
        pcStatusMessage = "OK: No errors";
    }
}
/*-----------------------------------------------------------*/

static void prvTestTask(void *pvParameters)
{
    const unsigned long ulMSToSleep = 5;

    /* Just to remove compiler warnings. */
    (void)pvParameters;

    /* This task is just used to test the eTaskStateGet() function.  It
     * does not have anything to do. */
    for (;;)
    {
        /* Sleep to reduce CPU load, but don't sleep indefinitely in case there are
         * tasks waiting to be terminated by the idle task. */
        struct timespec ts;
        ts.tv_sec  = ulMSToSleep / 1000;
        ts.tv_nsec = ulMSToSleep % 1000l * 1000000l;
        nanosleep(&ts, NULL);
    }
}
/*-----------------------------------------------------------*/

/* Called from vApplicationIdleHook(), which is defined in main.c. */
void vFullDemoIdleFunction(void)
{
    const unsigned long ulMSToSleep = 15;
    void *pvAllocated;

    /* Sleep to reduce CPU load, but don't sleep indefinitely in case there are
     * tasks waiting to be terminated by the idle task. */
    struct timespec ts;

    ts.tv_sec  = ulMSToSleep / 1000;
    ts.tv_nsec = ulMSToSleep % 1000l * 1000000l;
    nanosleep(&ts, NULL);

    /* Demonstrate a few utility functions that are not demonstrated by any of
     * the standard demo tasks. */
    prvDemonstrateTaskStateAndHandleGetFunctions();

    /* Demonstrate the use of xTimerPendFunctionCall(), which is not
     * demonstrated by any of the standard demo tasks. */
    prvDemonstratePendingFunctionCall();

    /* Demonstrate the use of functions that query information about a software
     * timer. */
    prvDemonstrateTimerQueryFunctions();

    /* If xMutexToDelete has not already been deleted, then delete it now.
     * This is done purely to demonstrate the use of, and test, the
     * vSemaphoreDelete() macro.  Care must be taken not to delete a semaphore
     * that has tasks blocked on it. */
    if (xMutexToDelete != NULL)
    {
        /* For test purposes, add the mutex to the registry, then remove it
         * again, before it is deleted - checking its name is as expected before
         * and after the assertion into the registry and its removal from the
         * registry. */
        configASSERT(pcQueueGetName(xMutexToDelete) == NULL);
        vQueueAddToRegistry(xMutexToDelete, "Test_Mutex");
        configASSERT(strcmp(pcQueueGetName(xMutexToDelete), "Test_Mutex") == 0);
        vQueueUnregisterQueue(xMutexToDelete);
        configASSERT(pcQueueGetName(xMutexToDelete) == NULL);

        vSemaphoreDelete(xMutexToDelete);
        xMutexToDelete = NULL;
    }

    /* Exercise heap_5 a bit.  The malloc failed hook will trap failed
     * allocations so there is no need to test here. */
    pvAllocated = pvPortMalloc((rand() % 500) + 1);
    vPortFree(pvAllocated);

/* Exit after a fixed time so code coverage results are written to the
 * disk. */
#if (projCOVERAGE_TEST == 1)
    {
        const TickType_t xMaxRunTime = pdMS_TO_TICKS(30000UL);

        /* Exercise code not otherwise executed by standard demo/test tasks. */
        if (xRunCodeCoverageTestAdditions() != pdPASS)
        {
            pcStatusMessage = "Code coverage additions failed.\r\n";
            xErrorCount++;
        }

        if ((xTaskGetTickCount() - configINITIAL_TICK_COUNT) >= xMaxRunTime) { vTaskEndScheduler(); }
    }
#endif /* if ( projCOVERAGE_TEST == 1 ) */
}
/*-----------------------------------------------------------*/

/* Called by vApplicationTickHook(), which is defined in main.c. */
void vFullDemoTickHookFunction(void)
{
    TaskHandle_t xTimerTask;

/* Call the periodic timer test, which tests the timer API functions that
 * can be called from an ISR. */
#if (configUSE_PREEMPTION != 0)
    {
        /* Only created when preemption is used. */
        vTimerPeriodicISRTests();
    }
#endif

    /* Call the periodic queue overwrite from ISR demo. */
    vQueueOverwritePeriodicISRDemo();

#if (configUSE_QUEUE_SETS == 1) /* Remove the tests if queue sets are not defined. */
    {
        /* Write to a queue that is in use as part of the queue set demo to
         * demonstrate using queue sets from an ISR. */
        vQueueSetAccessQueueSetFromISR();
        vQueueSetPollingInterruptAccess();
    }
#endif

    /* Exercise event groups from interrupts. */
    vPeriodicEventGroupsProcessing();

    /* Exercise giving mutexes from an interrupt. */
    vInterruptSemaphorePeriodicTest();

    /* Exercise using task notifications from an interrupt. */
    xNotifyTaskFromISR();
    /* xNotifyArrayTaskFromISR(); */

    /* Writes to stream buffer byte by byte to test the stream buffer trigger
     * level functionality. */
    vPeriodicStreamBufferProcessing();

    /* Writes a string to a string buffer four bytes at a time to demonstrate
     * a stream being sent from an interrupt to a task. */
    vBasicStreamBufferSendFromISR();

    /* For code coverage purposes. */
    xTimerTask = xTimerGetTimerDaemonTaskHandle();
    configASSERT(uxTaskPriorityGetFromISR(xTimerTask) == configTIMER_TASK_PRIORITY);
}
/*-----------------------------------------------------------*/

static void prvPendedFunction(void *pvParameter1, uint32_t ulParameter2)
{
    static intptr_t ulLastParameter1 = 1000UL, ulLastParameter2 = 0UL;
    intptr_t ulParameter1;

    ulParameter1 = (intptr_t)pvParameter1;

    /* Ensure the parameters are as expected. */
    configASSERT(ulParameter1 == (ulLastParameter1 + 1));
    configASSERT(ulParameter2 == (ulLastParameter2 + 1));

    /* Remember the parameters for the next time the function is called. */
    ulLastParameter1 = ulParameter1;
    ulLastParameter2 = ulParameter2;

    /* Remove compiler warnings in case configASSERT() is not defined. */
    (void)ulLastParameter1;
    (void)ulLastParameter2;
}
/*-----------------------------------------------------------*/

static void prvTestTimerCallback(TimerHandle_t xTimer)
{
    /* This is the callback function for the timer accessed by
     * prvDemonstrateTimerQueryFunctions().  The callback does not do anything. */
    (void)xTimer;
}
/*-----------------------------------------------------------*/

static void prvDemonstrateTimerQueryFunctions(void)
{
    static TimerHandle_t xTimer = NULL;
    const char *pcTimerName     = "TestTimer";
    volatile TickType_t xExpiryTime;
    const TickType_t xDontBlock = 0;

    if (xTimer == NULL)
    {
        xTimer = xTimerCreate(pcTimerName, portMAX_DELAY, pdTRUE, NULL, prvTestTimerCallback);

        if (xTimer != NULL)
        {
            /* Called from the idle task so a block time must not be
             * specified. */
            xTimerStart(xTimer, xDontBlock);
        }
    }

    if (xTimer != NULL)
    {
        /* Demonstrate querying a timer's name. */
        configASSERT(strcmp(pcTimerGetName(xTimer), pcTimerName) == 0);

        /* Demonstrate querying a timer's period. */
        configASSERT(xTimerGetPeriod(xTimer) == portMAX_DELAY);

        /* Demonstrate querying a timer's next expiry time, although nothing is
         * done with the returned value.  Note if the expiry time is less than the
         * maximum tick count then the expiry time has overflowed from the current
         * time.  In this case the expiry time was set to portMAX_DELAY, so it is
         * expected to be less than the current time until the current time has
         * itself overflowed. */
        xExpiryTime = xTimerGetExpiryTime(xTimer);
        (void)xExpiryTime;
    }
}
/*-----------------------------------------------------------*/

static void prvDemonstratePendingFunctionCall(void)
{
    static intptr_t ulParameter1 = 1000UL, ulParameter2 = 0UL;
    const TickType_t xDontBlock = 0; /* This is called from the idle task so must *not* attempt to block. */

    /* prvPendedFunction() just expects the parameters to be incremented by one
     * each time it is called. */

    ulParameter1++;
    ulParameter2++;

    /* Pend the function call, sending the parameters. */
    xTimerPendFunctionCall(prvPendedFunction, (void *)ulParameter1, ulParameter2, xDontBlock);
}
/*-----------------------------------------------------------*/

static void prvDemonstrateTaskStateAndHandleGetFunctions(void)
{
    TaskHandle_t xIdleTaskHandle, xTimerTaskHandle;
    char *pcTaskName;
    static portBASE_TYPE xPerformedOneShotTests = pdFALSE;
    TaskHandle_t xTestTask;
    TaskStatus_t xTaskInfo;
    extern StackType_t uxTimerTaskStack[];

    /* Demonstrate the use of the xTimerGetTimerDaemonTaskHandle() and
     * xTaskGetIdleTaskHandle() functions.  Also try using the function that sets
     * the task number. */
    xIdleTaskHandle  = xTaskGetIdleTaskHandle();
    xTimerTaskHandle = xTimerGetTimerDaemonTaskHandle();

    /* This is the idle hook, so the current task handle should equal the
     * returned idle task handle. */
    if (xTaskGetCurrentTaskHandle() != xIdleTaskHandle)
    {
        pcStatusMessage = "Error:  Returned idle task handle was incorrect";
        xErrorCount++;
    }

    /* Check the same handle is obtained using the idle task's name.  First try
     * with the wrong name, then the right name. */
    if (xTaskGetHandle("Idle") == xIdleTaskHandle)
    {
        pcStatusMessage = "Error:  Returned handle for name Idle was incorrect";
        xErrorCount++;
    }

    if (xTaskGetHandle("IDLE") != xIdleTaskHandle)
    {
        pcStatusMessage = "Error:  Returned handle for name Idle was incorrect";
        xErrorCount++;
    }

    /* Check the timer task handle was returned correctly. */
    pcTaskName = pcTaskGetName(xTimerTaskHandle);

    if (strcmp(pcTaskName, "Tmr Svc") != 0)
    {
        pcStatusMessage = "Error:  Returned timer task handle was incorrect";
        xErrorCount++;
    }

    if (xTaskGetHandle("Tmr Svc") != xTimerTaskHandle)
    {
        pcStatusMessage = "Error:  Returned handle for name Tmr Svc was incorrect";
        xErrorCount++;
    }

    /* This task is running, make sure it's state is returned as running. */
    if (eTaskStateGet(xIdleTaskHandle) != eRunning)
    {
        pcStatusMessage = "Error:  Returned idle task state was incorrect";
        xErrorCount++;
    }

    /* If this task is running, then the timer task must be blocked. */
    if (eTaskStateGet(xTimerTaskHandle) != eBlocked)
    {
        pcStatusMessage = "Error:  Returned timer task state was incorrect";
        xErrorCount++;
    }

    /* Also with the vTaskGetInfo() function. */
    // vTaskGetInfo(xTimerTaskHandle, /* The task being queried. */
    //              &xTaskInfo,       /* The structure into which information on the task
    //                                   will be written. */
    //              pdTRUE,           /* Include the task's high watermark in the structure. */
    //              eInvalid);        /* Include the task state in the structure. */

    /* Check the information returned by vTaskGetInfo() is as expected. */
    if ((xTaskInfo.eCurrentState != eBlocked) || (strcmp(xTaskInfo.pcTaskName, "Tmr Svc") != 0)
        || (xTaskInfo.uxCurrentPriority != configTIMER_TASK_PRIORITY) || (xTaskInfo.pxStackBase != uxTimerTaskStack)
        || (xTaskInfo.xHandle != xTimerTaskHandle))
    {
        pcStatusMessage = "Error:  vTaskGetInfo() returned incorrect information "
                          "about the timer task";
        xErrorCount++;
    }

    /* Other tests that should only be performed once follow.  The test task
     * is not created on each iteration because to do so would cause the death
     * task to report an error (too many tasks running). */
    if (xPerformedOneShotTests == pdFALSE)
    {
        /* Don't run this part of the test again. */
        xPerformedOneShotTests = pdTRUE;

        /* Create a test task to use to test other eTaskStateGet() return values. */
        if (xTaskCreate(prvTestTask, "Test", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xTestTask) == pdPASS)
        {
            /* If this task is running, the test task must be in the ready state. */
            if (eTaskStateGet(xTestTask) != eReady)
            {
                pcStatusMessage = "Error: Returned test task state was incorrect 1";
                xErrorCount++;
            }

            /* Now suspend the test task and check its state is reported correctly. */
            vTaskSuspend(xTestTask);

            if (eTaskStateGet(xTestTask) != eSuspended)
            {
                pcStatusMessage = "Error: Returned test task state was incorrect 2";
                xErrorCount++;
            }

            /* Now delete the task and check its state is reported correctly. */
            vTaskDelete(xTestTask);

            if (eTaskStateGet(xTestTask) != eDeleted)
            {
                pcStatusMessage = "Error: Returned test task state was incorrect 3";
                xErrorCount++;
            }
        }
    }
}
/*-----------------------------------------------------------*/

static void prvDemoQueueSpaceFunctions(void *pvParameters)
{
    QueueHandle_t xQueue                       = NULL;
    const unsigned portBASE_TYPE uxQueueLength = 10;
    unsigned portBASE_TYPE uxReturn, x;

    /* Remove compiler warnings. */
    (void)pvParameters;

    /* Create the queue that will be used.  Nothing is actually going to be
     * sent or received so the queue item size is set to 0. */
    xQueue = xQueueCreate(uxQueueLength, 0);
    configASSERT(xQueue);

    for (;;)
    {
        for (x = 0; x < uxQueueLength; x++)
        {
            /* Ask how many messages are available... */
            uxReturn = uxQueueMessagesWaiting(xQueue);

            /* Check the number of messages being reported as being available
             * is as expected, and force an assert if not. */
            if (uxReturn != x)
            {
                /* xQueue cannot be NULL so this is deliberately causing an
                 * assert to be triggered as there is an error. */
                configASSERT(xQueue == NULL);
            }

            /* Ask how many spaces remain in the queue... */
            uxReturn = uxQueueSpacesAvailable(xQueue);

            /* Check the number of spaces being reported as being available
             * is as expected, and force an assert if not. */
            if (uxReturn != (uxQueueLength - x))
            {
                /* xQueue cannot be NULL so this is deliberately causing an
                 * assert to be triggered as there is an error. */
                configASSERT(xQueue == NULL);
            }

            /* Fill one more space in the queue. */
            xQueueSendToBack(xQueue, NULL, 0);
        }

        /* Perform the same check while the queue is full. */
        uxReturn = uxQueueMessagesWaiting(xQueue);

        if (uxReturn != uxQueueLength) { configASSERT(xQueue == NULL); }

        uxReturn = uxQueueSpacesAvailable(xQueue);

        if (uxReturn != 0) { configASSERT(xQueue == NULL); }

        /* The queue is full, start again. */
        xQueueReset(xQueue);

#if (configUSE_PREEMPTION == 0)
        taskYIELD();
#endif
    }
}
/*-----------------------------------------------------------*/

static void prvPermanentlyBlockingSemaphoreTask(void *pvParameters)
{
    SemaphoreHandle_t xSemaphore;

    /* Prevent compiler warning about unused parameter in the case that
     * configASSERT() is not defined. */
    (void)pvParameters;

    /* This task should block on a semaphore, and never return. */
    xSemaphore = xSemaphoreCreateBinary();
    configASSERT(xSemaphore);

    xSemaphoreTake(xSemaphore, portMAX_DELAY);

    /* The above xSemaphoreTake() call should never return, force an assert if
     * it does. */
    configASSERT(pvParameters != NULL);
    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

static void prvPermanentlyBlockingNotificationTask(void *pvParameters)
{
    /* Prevent compiler warning about unused parameter in the case that
     * configASSERT() is not defined. */
    (void)pvParameters;

    /* This task should block on a task notification, and never return. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    /* The above ulTaskNotifyTake() call should never return, force an assert
     * if it does. */
    configASSERT(pvParameters != NULL);
    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

static void prvReloadModeTestTimerCallback(TimerHandle_t xTimer)
{
    intptr_t ulTimerID;

    /* Increment the timer's ID to show the callback has executed. */
    ulTimerID = (intptr_t)pvTimerGetTimerID(xTimer);
    ulTimerID++;
    vTimerSetTimerID(xTimer, (void *)ulTimerID);
}
/*-----------------------------------------------------------*/

static void prvDemonstrateChangingTimerReloadMode(void *pvParameters)
{
    TimerHandle_t xTimer;
    const char *const pcTimerName = "TestTimer";
    const TickType_t x100ms       = pdMS_TO_TICKS(100UL);

    /* Avoid compiler warnings about unused parameter. */
    (void)pvParameters;

    xTimer = xTimerCreate(pcTimerName, x100ms, pdFALSE, /* Created as a one-shot timer. */
                          0, prvReloadModeTestTimerCallback);
    configASSERT(xTimer);
    configASSERT(xTimerIsTimerActive(xTimer) == pdFALSE);
    configASSERT(xTimerGetTimerDaemonTaskHandle() != NULL);
    configASSERT(strcmp(pcTimerName, pcTimerGetName(xTimer)) == 0);
    configASSERT(xTimerGetPeriod(xTimer) == x100ms);

    /* Timer was created as a one-shot timer.  Its callback just increments the
     * timer's ID - so set the ID to 0, let the timer run for a number of timeout
     * periods, then check the timer has only executed once. */
    vTimerSetTimerID(xTimer, (void *)0);
    xTimerStart(xTimer, portMAX_DELAY);
    vTaskDelay(3UL * x100ms);
    configASSERT(((uintptr_t)(pvTimerGetTimerID(xTimer))) == 1UL);

    /* Now change the timer to be an auto-reload timer and check it executes
     * the expected number of times. */
    vTimerSetReloadMode(xTimer, pdTRUE);
    vTimerSetTimerID(xTimer, (void *)0);
    xTimerStart(xTimer, 0);
    vTaskDelay((3UL * x100ms) + (x100ms / 2UL)); /* Three full periods. */
    configASSERT((uintptr_t)(pvTimerGetTimerID(xTimer)) == 3UL);
    configASSERT(xTimerStop(xTimer, 0) != pdFAIL);

    /* Now change the timer back to be a one-shot timer and check it only
     * executes once. */
    vTimerSetReloadMode(xTimer, pdFALSE);
    vTimerSetTimerID(xTimer, (void *)0);
    xTimerStart(xTimer, 0);
    vTaskDelay(3UL * x100ms);
    configASSERT(xTimerStop(xTimer, 0) != pdFAIL);
    configASSERT((uintptr_t)(pvTimerGetTimerID(xTimer)) == 1UL);

    /* Clean up at the end. */
    xTimerDelete(xTimer, portMAX_DELAY);
    vTaskDelete(NULL);
}

void init_application()
{
    /* Initialise the trace recorder.  Use of the trace recorder is optional.
     * See http://www.FreeRTOS.org/trace for more information. */
    vTraceEnable(TRC_START);

    /* Start the trace recording - the recording is written to a file if
     * configASSERT() is called. */
    // printf( "\r\nTrace started.\r\nThe trace will be dumped to disk if a call
    // to configASSERT() fails.\r\n");

#if (TRACE_ON_ENTER == 1)
    printf("\r\nThe trace will be dumped to disk if Enter is hit.\r\n");
#endif
    uiTraceStart();

    console_init();
}
