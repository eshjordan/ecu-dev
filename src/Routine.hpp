#pragma once

#include "RTOS.hpp"
#include <string>
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
    const std::string m_name{};

    /** @brief Frequency of Routine's timer (Hz). */
    const double m_frequency{};

    friend class RoutineManager;

public:
    /**
     * @brief Construct a new Routine object. Must have a name and frequency.
     *
     * @param name Name of the routine, used for debugging.
     * @param frequency Frequency the routine is called, in Hz.
     */
    Routine(std::string name, const double &frequency) : m_name(std::move(name)), m_frequency(frequency)
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
template <class RoutineClass> class RoutineFactory : RoutineFactoryBase
{
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
    RoutineClass *create_routine(void) override { return new RoutineClass; }
};

} // namespace Impl
} // namespace System

// clang-format off
#define REGISTER_ROUTINE(name, frequency)                                                                                                           \
namespace System {                                                                                                                                  \
namespace Generated {                                                                                                                                    \
class name##_t : public System::Impl::Routine                                                                                                       \
{                                                                                                                                                   \
public:                                                                                                                                             \
    name##_t(void) : System::Impl::Routine((#name), (frequency)) {}                                                                                 \
                                                                                                                                                    \
private:                                                                                                                                            \
    static void FunctionBody(TimerHandle_t xTimer);                                                                                                 \
    const static System::Impl::Routine *const result_;                                                                                              \
    friend class System::Impl::RoutineManager;                                                                                                      \
};                                                                                                                                                  \
const System::Impl::Routine *const name##_t::result_ = System::Impl::RoutineManager::register_routine(new System::Impl::RoutineFactory<name##_t>);  \
}                                                                                                                                                   \
}                                                                                                                                                   \
void System::Generated::name##_t::FunctionBody(TimerHandle_t xTimer)
// clang-format on
