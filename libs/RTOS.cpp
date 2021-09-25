#include "RTOS.hpp"
// #include "../src/Server.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unistd.h>

/**
 * @brief When configSUPPORT_STATIC_ALLOCATION is set to 1 the application
 * writer can use a callback function to optionally provide the memory required
 * by the idle and timer tasks.  This is the stack that will be used by the
 * timer task.  It is declared here, as a global, so it can be checked by a test
 * that is implemented in a different file.
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

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  (void)pcTaskName;
  (void)pxTask;

  vAssertCalled(__FILE__, __LINE__);
}

void vApplicationDaemonTaskStartupHook(void) {}

void vApplicationTickHook(void) {}

void vAssertCalled(const char *const pcFileName, unsigned long ulLine) {
  volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

  (void)ulLine;
  (void)pcFileName;

  taskENTER_CRITICAL();
  {
    char str[256];
    printf("Assertion failed in %s at line %lu\n", pcFileName, ulLine);

    /* You can step out of this function to debug the assertion by using
     * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
     * value. */
    while (ulSetToNonZeroInDebuggerToContinue == (uint32_t)0) {
      __asm volatile("NOP");
      __asm volatile("NOP");
    }
  }
  taskEXIT_CRITICAL();
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
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

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
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
