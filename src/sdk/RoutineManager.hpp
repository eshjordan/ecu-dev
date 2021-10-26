#pragma once

#include "Routine.hpp"
#include <cstdint>
#include <cstring>

namespace System {
namespace Impl {

/**
 * @brief Singleton record of all managed Routines. Manages all user-defined auto-generated routines.
 *
 */
class RoutineManager
{

private:
    /** Member variables */

    /**
     * @brief List of routines to be executed.
     *
     */
    static Routine *m_routines[];

    /**
     * @brief List of tasks under execution.
     *
     */
    static TaskHandle_t *m_tasks[];

    /**
     * @brief List of timers under execution.
     *
     */
    static TimerHandle_t *m_timers[];

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
    template <typename T> static Routine *register_routine(RoutineFactory<T> *factory)
    {
        /* Register Routine */
        T *rout = factory->create_routine();

        RoutineManager::m_routines[RoutineManager::m_routines_count] = rout;
        RoutineManager::m_routines_count++;

        /* Register its Task */

        char task_name[128]  = "";
        char timer_name[128] = "";
        strncat(strncpy(task_name, rout->m_name.data(), sizeof(task_name)), "_task",
                sizeof(task_name) - strlen(task_name));
        strncat(strncpy(timer_name, rout->m_name.data(), sizeof(timer_name)), "_timer",
                sizeof(timer_name) - strlen(timer_name));

        TaskHandle_t task = nullptr;
        xTaskCreate(rout->task_cb,    /* Function that implements the task. */
                    task_name,        /* Name of the task. */
                    10000,            /* Stack size in words, not bytes. */
                    nullptr,          /* Parameter passed into the task. */
                    tskIDLE_PRIORITY, /* Priority of the task. */
                    &task             /* Pointer to the task's handle. */
        );

        rout->task_handle = task;

        RoutineManager::m_tasks[RoutineManager::m_tasks_count] = &rout->task_handle;
        RoutineManager::m_tasks_count++;

        /* Register its Timer */

        const auto period      = (size_t)(1000.0 / rout->m_frequency);
        const TickType_t ticks = pdMS_TO_TICKS(period);

        rout->timer_handle = xTimerCreate(timer_name,    /* The text name assigned to the software timer - for
                                                                      debug only as it is not used by the kernel. */
                                          ticks,         /* The period of the software timer in ticks. */
                                          true,          /* xAutoReload is set to pdTRUE. */
                                          nullptr,       /* The timer's ID is not used. */
                                          rout->timer_cb /* The function executed when the timer expires. */
        );

        RoutineManager::m_timers[RoutineManager::m_timers_count] = &rout->timer_handle;
        RoutineManager::m_timers_count++;

        xTimerStart(rout->timer_handle, 0);

        return rout;
    }
};

} // namespace Impl
} // namespace System
