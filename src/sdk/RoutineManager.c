#include "string.h"
#include "types.h"
#include "RoutineManager.h"

TaskHandle_t m_routines[128] = {0};
TaskHandle_t m_tasks[128] = {0};
TimerHandle_t m_timers[128] = {0};
uint32_t m_routines_count = 0;
uint32_t m_tasks_count = 0;
uint32_t m_timers_count = 0;


int register_routine(const char *name, uint32_t frequency,
                TaskFunction_t function, StackType_t *stack,
                uint32_t stack_size,
                StaticTask_t *task_storage,
                TaskHandle_t **task_handle,
                TimerCallbackFunction_t timer_callback,
                StaticTimer_t *timer_storage)
{
    /* Register Routine */
    *task_handle = &m_tasks[m_tasks_count];

    **task_handle = xTaskCreateStatic(
        function, /* Function that implements the task. */
        NULL, /* Name of the task. */
        stack_size / 2, /* Stack size in words, not bytes. */
        NULL, /* Parameter passed into the task. */
        tskIDLE_PRIORITY, /* Priority of the task. */
        stack, /* Pointer to the task's stack. */
        task_storage /* Pointer to the task's storage. */
    );

    m_tasks_count++;

    /* Register its Timer */

    const size_t period = (size_t)(1000.0 / frequency);
    const TickType_t ticks = pdMS_TO_TICKS(period);

    TimerHandle_t *const timer_handle = &m_timers[m_timers_count];

    *timer_handle = xTimerCreateStatic(
        name, /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
        ticks, /* The period of the software timer in ticks. */
        TRUE, /* xAutoReload is set to pdTRUE. */
        NULL, /* The timer's ID is not used. */
        timer_callback, /* The function executed when the timer expires. */
        timer_storage /* Pointer to the timer's storage. */
    );

    m_timers_count++;

    if (xTimerStart(*timer_handle, 0) != pdTRUE) {
        printf("Timer %s start failed!", name);
    }

    return 0;
}

