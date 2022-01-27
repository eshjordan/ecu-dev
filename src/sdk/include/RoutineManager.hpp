#pragma once

#include "RTOS.hpp"
#include "portmacro.h"
#include <cstdint>
#include <cstring>

using RoutineFunction_t = TaskFunction_t;

namespace System
{
namespace Impl
{

/**
 * @brief Singleton record of all managed Routines. Manages all user-defined auto-generated routines.
 *
 */
class RoutineManager {
    private:
	/** Member variables */

	/**
     * @brief List of routines to be executed.
     *
     */
	static TaskHandle_t m_routines[];

	/**
     * @brief List of tasks under execution.
     *
     */
	static TaskHandle_t m_tasks[];

	/**
     * @brief List of timers under execution.
     *
     */
	static TimerHandle_t m_timers[];

	/**
     * @brief The number of routines in the system.
     *
     */
	static uint32_t m_routines_count;

	/**
     * @brief The number of tasks in the system.
     *
     */
	static uint32_t m_tasks_count;

	/**
     * @brief The number of timers in the system.
     *
     */
	static uint32_t m_timers_count;

    public:
	/**
     * @brief Register a block of code to be executed by a task on a timer. Used by the REGISTER_ROUTINE macro.
     *
     * @tparam T Custom class unique to this routine, as defined within the REGISTER_ROUTINE macro.
     * @param name Name of the routine (for debugging purposes).
     * @param factory Used to create an instance of the generated class, which contains the FunctionBody.
     * @return Routine* Pointer to the created routine (as Routine class).
     */
	static int register_routine(const char *name, uint32_t frequency,
				    TaskFunction_t function, StackType_t *stack,
				    uint32_t stack_size,
				    StaticTask_t *task_storage,
				    TaskHandle_t **task_handle,
				    TimerCallbackFunction_t timer_callback,
				    StaticTimer_t *timer_storage)
	{
		/* Register Routine */
		*task_handle =
			&RoutineManager::m_tasks[RoutineManager::m_tasks_count];

		**task_handle = xTaskCreateStatic(
			function, /* Function that implements the task. */
			NULL, /* Name of the task. */
			stack_size / 2, /* Stack size in words, not bytes. */
			nullptr, /* Parameter passed into the task. */
			tskIDLE_PRIORITY, /* Priority of the task. */
			stack, /* Pointer to the task's stack. */
			task_storage /* Pointer to the task's storage. */
		);

		RoutineManager::m_tasks_count++;

		/* Register its Timer */

		const auto period = (size_t)(1000.0 / frequency);
		const TickType_t ticks = pdMS_TO_TICKS(period);

		TimerHandle_t *const timer_handle =
			&RoutineManager::m_timers[RoutineManager::m_timers_count];

		*timer_handle = xTimerCreateStatic(
			name, /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
			ticks, /* The period of the software timer in ticks. */
			true, /* xAutoReload is set to pdTRUE. */
			nullptr, /* The timer's ID is not used. */
			timer_callback, /* The function executed when the timer expires. */
			timer_storage /* Pointer to the timer's storage. */
		);

		RoutineManager::m_timers_count++;

		if (xTimerStart(*timer_handle, 0) != pdTRUE) {
			printf("Timer %s start failed!", name);
		}

		return 0;
	}
};

} // namespace Impl
} // namespace System
