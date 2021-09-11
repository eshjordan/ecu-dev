#pragma once

#include "RTOS.hpp"
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
     * @brief The number of routines in the system.
     *
     */
    uint32_t m_timers_count{};

public:
    /**
     * @brief Register a block of code to be executed on a timer. Used by the REGISTER_ROUTINE macro.
     *
     * @tparam T Custom class unique to this routine, as defined within the REGISTER_ROUTINE macro.
     * @param name Name of the routine (for debugging purposes).
     * @param factory Used to create an instance of the generated class, which contains the FunctionBody.
     * @return Routine* Pointer to the created routine (as Routine class).
     */
    template <typename T> static Routine *register_routine(RoutineFactory<T> *factory)
    {
        RoutineManager *manager = RoutineManager::get_instance();
        T *rout                 = factory->create_routine();

        TimerCallbackFunction_t callback = T::FunctionBody;

        const auto period      = (size_t)(1000.0 / rout->m_frequency);
        const TickType_t ticks = pdMS_TO_TICKS(period);

        TimerHandle_t x_timer = xTimerCreate(rout->m_name.c_str(), /* The text name assigned to the software timer - for
                                                                      debug only as it is not used by the kernel. */
                                             ticks,                /* The period of the software timer in ticks. */
                                             true,                 /* xAutoReload is set to pdTRUE. */
                                             nullptr,              /* The timer's ID is not used. */
                                             callback              /* The function executed when the timer expires. */
        );

        manager->m_routines[manager->m_routines_count] = rout;
        manager->m_timers[manager->m_timers_count]     = x_timer;

        manager->m_routines_count++;
        manager->m_timers_count++;

        xTimerStart(x_timer, 0);

        return rout;
    }
};

} // namespace Impl
} // namespace System
