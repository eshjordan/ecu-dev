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
    const std::string m_name{};
    const double m_frequency{};
    friend class RoutineManager;

public:
    /**
     * @brief Construct a new Routine object. Must have a name and frequency.
     *
     * @param name Name of the routine, used for debugging.
     * @param frequency Frequency the routine is called, in Hz.
     */
    Routine(std::string name, const double &frequency) : m_name(std::move(name)), m_frequency(frequency) {}

    ~Routine(void) = default;
};

class RoutineFactoryBase
{
public:
    RoutineFactoryBase(void) = default;

    ~RoutineFactoryBase(void) = default;

    /**
     * @brief Creates a test instance to run. The instance is both created and destroyed within TestInfoImpl::Run()
     *
     * @return Routine*
     */
    virtual Routine *create_routine(void) = 0;
};

template <class RoutineClass> class RoutineFactory : RoutineFactoryBase
{
public:
    RoutineFactory(void) : RoutineFactoryBase() {}

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
