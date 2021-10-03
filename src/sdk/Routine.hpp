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

protected:
    /** @brief Routine's timer handle, used to signal the task to continue executing. */
    xTimerHandle timer_handle{};

    /** @brief Routine's task handle, used to call the user-defined function. */
    xTaskHandle task_handle{};

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

#define REGISTER_ROUTINE(name, frequency) \
 /** \
 * @brief What's going on here? Based on the macro arguments (name and frequency), this macro auto-generates a new \
 *      Routine. First the class definition, inheriting from System::Impl::Routine. Static functions: \
 *          FunctionBody - The user-defined function to call at the specified frequency. \
 *          timer_cb - This is the actual function that is called on the xTimer. We do this because the body of a \
 *              timer cb is not allowed to block, all this does is signal the task_cb to continue. \
 *          task_cb - This function just loops infinitely, whenever signaled by the timer, and calls the user-defined \
 *              function on each iteration. \
 *      We then call System::Impl::RoutineManager::register_routine to generate an instance of our auto-defined class, \
 *      and set up the task and timer. The instance is stored in the static result_ member variable, as a Routine \
 *      (parent) pointer. The last line is left as a hanging function declaration, and the user's function body \
 *      below the macro is used. \
 * \
 */ \
namespace System { \
namespace Generated { \
class name##_t : public System::Impl::Routine \
{ \
public: \
    name##_t(void) : System::Impl::Routine((#name), (frequency)) {} \
 \
    static void FunctionBody(void); \
 \
    static void task_cb(void *parameters); \
 \
    static void timer_cb(TimerHandle_t xTimer); \
 \
private: \
    const static System::Impl::Routine *const result_; \
    friend class System::Impl::RoutineManager; \
}; \
} /* namespace Generated */ \
} /* namespace System */ \
 \
void System::Generated::name##_t::task_cb(void *parameters) \
{ \
    while (true) \
    { \
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY); \
 \
        System::Generated::name##_t::FunctionBody(); \
    } \
} \
 \
void System::Generated::name##_t::timer_cb(TimerHandle_t xTimer) \
{ \
    auto res = (name##_t *)result_; \
    xTaskNotify(res->task_handle, 0, eNoAction); \
} \
 \
const System::Impl::Routine *const System::Generated::name##_t::result_ = \
    System::Impl::RoutineManager::register_routine(new System::Impl::RoutineFactory<name##_t>); \
void System::Generated::name##_t::FunctionBody()

// clang-format on

#define GET_FN_PTR_OF_ROUTINE(name) System::Generated::name##_t::FunctionBody
