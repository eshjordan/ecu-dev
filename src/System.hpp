#pragma once

#include "RTOS.hpp"
#include "Routine.hpp"

class System
{
public:
    static System *get_instance(void)
    {
        static System instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
        return &instance;
    }

private:
    /**
     * @brief Construct a System instance. Private to force singleton.
     *
     */
    System(void) {}

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
     * @brief Copy constructor does nothing, because the class is a singleton.
     *
     */
    System(System const &other) {}

    /**
     * @brief Copy assignment operator does nothing, because the class is a singleton.
     *
     */
    void operator=(System const &other) {}

    template <typename T> static Routine *register_routine(const std::string &name, RoutineFactory<T> *factory)
    {
        System *sys = System::get_instance();
        T *rout     = factory->create_routine();

        TimerCallbackFunction_t cb = T::FunctionBody;

        const uint16_t period = 1000 / (float)rout->m_frequency;

        const TickType_t ticks = pdMS_TO_TICKS(period);

        TimerHandle_t x_timer = xTimerCreate(name.c_str(), /* The text name assigned to the software timer - for
                                                              debug only as it is not used by the kernel. */
                                             ticks,        /* The period of the software timer in ticks. */
                                             true,         /* xAutoReload is set to pdTRUE. */
                                             nullptr,      /* The timer's ID is not used. */
                                             cb            /* The function executed when the timer expires. */
        );

        sys->m_routines[sys->m_routines_count] = rout;
        sys->m_timers[sys->m_timers_count]     = x_timer;

        sys->m_routines_count++;
        sys->m_timers_count++;

        xTimerStart(x_timer, 0);

        return rout;
    }
};
