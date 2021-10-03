#pragma once

#include "Routine.hpp"
#include <cstdint>

namespace System {
namespace Impl {

/**
 * @brief Singleton record of all managed Routines. Manages all user-defined auto-generated routines.
 *
 */
class RoutineManager
{

private:
    /**
     * @brief Get the single instance of the System object. Shouldn't be accessed outside the class.
     *
     * @return System* Pointer to the System object.
     */
    static RoutineManager *get_instance(void)
    {
        static RoutineManager instance; // Guaranteed to be destroyed.
                                        // Instantiated on first use.
        return &instance;
    }

    /** Rule of Six */

    /**
     * @brief Default constructor. Construct the RoutineManager instance. Private because the class is a singleton.
     *
     */
    RoutineManager(void) = default;

    /**
     * @brief Destructor. Destroy the RoutineManager instance. Private because the class is a singleton.
     *
     */
    ~RoutineManager(void) = default;

public:
    /**
     * @brief Copy constructor. Deleted because the class is a singleton.
     *
     */
    RoutineManager(const RoutineManager &other) = delete;

    /**
     * @brief Copy assignment operator. Deleted because the class is a singleton.
     *
     */
    RoutineManager &operator=(RoutineManager const &other) = delete;

    /**
     * @brief Move constructor. Deleted because the class is a singleton.
     *
     * @param other Other System object to move.
     */
    RoutineManager(RoutineManager &&other) = delete;

    /**
     * @brief Move assignment operator. Deleted because the class is a singleton.
     *
     */
    RoutineManager &operator=(RoutineManager &&other) = delete;

private:
    /** Member variables */

    /**
     * @brief List of routines to be executed.
     *
     */
    Routine **m_routines = static_cast<Routine **>(pvPortMalloc(1000UL * sizeof(Routine *)));

    /**
     * @brief List of tasks under execution.
     *
     */
    TaskHandle_t *m_tasks = static_cast<TaskHandle_t *>(pvPortMalloc(1000UL * sizeof(TaskHandle_t)));

    /**
     * @brief List of timers under execution.
     *
     */
    TimerHandle_t *m_timers = static_cast<TimerHandle_t *>(pvPortMalloc(1000UL * sizeof(TimerHandle_t)));

    /**
     * @brief The number of routines in the system.
     *
     */
    uint32_t m_routines_count{};

    /**
     * @brief The number of tasks in the system.
     *
     */
    uint32_t m_tasks_count{};

    /**
     * @brief The number of timers in the system.
     *
     */
    uint32_t m_timers_count{};

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

        RoutineManager *manager = RoutineManager::get_instance();
        T *rout                 = factory->create_routine();

        manager->m_routines[manager->m_routines_count] = rout;
        manager->m_routines_count++;

        /* Register its Task */

        TaskHandle_t task = nullptr;
        xTaskCreate(rout->task_cb,                    /* Function that implements the task. */
                    (rout->m_name + "_task").c_str(), /* Name of the task. */
                    10000,                            /* Stack size in words, not bytes. */
                    nullptr,                          /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                 /* Priority of the task. */
                    &task                             /* Pointer to the task's handle. */
        );

        rout->task_handle = task;

        manager->m_tasks[manager->m_tasks_count] = rout->task_handle;
        manager->m_tasks_count++;

        /* Register its Timer */

        const auto period      = (size_t)(1000.0 / rout->m_frequency);
        const TickType_t ticks = pdMS_TO_TICKS(period);

        rout->timer_handle = xTimerCreate(rout->m_name.c_str(), /* The text name assigned to the software timer - for
                                                                   debug only as it is not used by the kernel. */
                                          ticks,                /* The period of the software timer in ticks. */
                                          true,                 /* xAutoReload is set to pdTRUE. */
                                          nullptr,              /* The timer's ID is not used. */
                                          rout->timer_cb        /* The function executed when the timer expires. */
        );

        manager->m_timers[manager->m_timers_count] = rout->timer_handle;
        manager->m_timers_count++;

        xTimerStart(rout->timer_handle, 0);

        return rout;
    }
};

} // namespace Impl
} // namespace System
