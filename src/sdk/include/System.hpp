#pragma once

#include "Routine.hpp"
#include "RoutineManager.hpp"
#include <string>
#include <vector>

/**
 * @brief Hide the implementation of the system from the user. Expose only the simplest part of the interface.
 *
 */
namespace System {

namespace Impl {

class ParameterList
{
public:
    template <typename T> static void add_parameter(const std::string &name, const T &value);
    template <typename T> static T get_parameter(const std::string &name);
    template <typename T> static void set_parameter(const std::string &name, const T &value);
};

static int s_argc = 0;

static std::vector<std::string> s_argv;

/** @brief Stop chat users from resetting everything accidentally. */
static bool initialised = false;

/** @brief The routine manager. */
static bool network_up = false;

/**
 * @brief Get the executable path object
 *
 * @return std::string Path to the executable, from argv[0]
 */
std::string get_executable_path();

} // namespace Impl

/**
 * @brief
 *
 */
void init(int argc, char **argv);

/**
 * @brief
 *
 */
void run(void);

/**
 * @brief
 *
 */
void restart(int signal);

/**
 * @brief
 *
 */
inline void restart(void *signal) { restart(*reinterpret_cast<int *>(signal)); }

/**
 * @brief
 *
 */
void shutdown(int signal);

/**
 * @brief Get the value of an existing parameter given its name.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @return T Current value of the Parameter.
 */
template <typename T> [[nodiscard]] T get_parameter(const std::string &name)
{
    return System::Impl::ParameterList::get_parameter<T>(name);
}

/**
 * @brief Set the value of an existing parameter given its name.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @param value New value of the Parameter.
 */
template <typename T> void set_parameter(const std::string &name, const T &value)
{
    System::Impl::ParameterList::set_parameter<T>(name, value);
}

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
    static System::Impl::RoutineFactory<name##_t> factory_; \
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
    auto *res = (name##_t *)result_; \
    xTaskNotify(res->task_handle, 0, eNoAction); \
} \
 \
 \
System::Impl::RoutineFactory<System::Generated::name##_t> System::Generated::name##_t::factory_{}; \
const System::Impl::Routine *const System::Generated::name##_t::result_ = \
    System::Impl::RoutineManager::register_routine(&System::Generated::name##_t::factory_); \
 \
void System::Generated::name##_t::FunctionBody()

// clang-format on

#define GET_FN_PTR_OF_ROUTINE(name) System::Generated::name##_t::FunctionBody
