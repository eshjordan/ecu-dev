#pragma once

#include "RTOS.h"
#include "types.h"
#include "portmacro.h"
#include <stdint.h>
#include <string.h>

typedef TaskFunction_t RoutineFunction_t;

/**
 * @brief Register a block of code to be executed by a task on a timer. Used by the REGISTER_ROUTINE macro.
 *
 * @tparam T Custom class unique to this routine, as defined within the REGISTER_ROUTINE macro.
 * @param name Name of the routine (for debugging purposes).
 * @param factory Used to create an instance of the generated class, which contains the FunctionBody.
 * @return Routine* Pointer to the created routine (as Routine class).
 */
int register_routine(const char *name, uint32_t frequency,
                TaskFunction_t function, StackType_t *stack,
                uint32_t stack_size,
                StaticTask_t *task_storage,
                TaskHandle_t **task_handle,
                TimerCallbackFunction_t timer_callback,
                StaticTimer_t *timer_storage);
