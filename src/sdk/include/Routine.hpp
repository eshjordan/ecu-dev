#pragma once

#include "RTOS.hpp"
#include <utility>

namespace System {
namespace Impl {

/**
 * @brief Base class for all generated routines.
 *
 */
class Routine
{
private:
    /** @brief Routine name. */
    const char *m_name{};

    /** @brief Frequency of Routine's timer (Hz). */
    const double m_frequency{};

    StaticTask_t m_task_storage;

    StackType_t m_task_stack[10000];

    StaticTimer_t m_timer_storage;

    friend class RoutineManager;

protected:
    /** @brief Routine's timer handle, used to signal the task to continue executing. */
    TimerHandle_t timer_handle{};

    /** @brief Routine's task handle, used to call the user-defined function. */
    TaskHandle_t task_handle{};

public:
    /**
     * @brief Construct a new Routine object. Must have a name and frequency.
     *
     * @param name Name of the routine, used for debugging.
     * @param frequency Frequency the routine is called, in Hz.
     */
    Routine(const char* name, const double &frequency) : m_name(name), m_frequency(frequency)
    {
        (void)name;
        (void)frequency;
    }

    /**
     * @brief Destroy the Routine object.
     *
     */
    ~Routine(void) = default;
};

/**
 * @brief Base class for Routine generators.
 *
 */
class RoutineFactoryBase
{
public:
    /**
     * @brief Construct a new Routine Factory Base object for each custom generator to inherit from.
     *
     */
    RoutineFactoryBase(void) = default;

    /**
     * @brief Destroy the Routine Factory Base object.
     *
     */
    ~RoutineFactoryBase(void) = default;

    /**
     * @brief Factory for making a newly generated Routine.
     *
     * @return Routine* Pointer to the newly generated Routine.
     */
    virtual Routine *create_routine(void) = 0;
};

/**
 * @brief Template Factory class to generate Routines.
 *
 * @tparam RoutineClass Type of Routine to generate (each is unique).
 */
template <class RoutineClass> class RoutineFactory : public RoutineFactoryBase
{
private:
    RoutineClass m_routine_class_instance{};

public:
    /**
     * @brief Construct a new Routine Factory object.
     *
     */
    RoutineFactory(void) : RoutineFactoryBase() {}

    /**
     * @brief Actually generate the Routine using its default constructor.
     *
     * @return RoutineClass* Pointer to the newly generated Routine.
     */
    RoutineClass *create_routine(void) override { return &m_routine_class_instance; }
};

} // namespace Impl
} // namespace System
