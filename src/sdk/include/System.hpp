#pragma once

#include "IO.hpp"
#include "ModuleManager.hpp"
#include "RoutineManager.hpp"
#include "ChannelList.hpp"
#include "ParameterList.hpp"

/**
 * @brief Hide the implementation of the system from the user. Expose only the simplest part of the interface.
 *
 */
namespace System
{

namespace Impl
{

static int s_argc = 0;

// static std::vector<const char *> s_argv;

/** @brief Stop chat users from resetting everything accidentally. */
static bool initialised = false;

/** @brief The routine manager. */
static bool network_up = false;

/**
 * @brief Get the executable path object
 *
 * @return const char * Path to the executable, from argv[0]
 */
const char *get_executable_path();

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
inline void restart(void *signal)
{
	restart(*reinterpret_cast<int *>(signal));
}

/**
 * @brief
 *
 */
void shutdown(int signal);

/**
 * @brief Add a new parameter to the server.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @param value Initial value of the Parameter.
 */
template <typename T> void add_parameter(const char *name, const T &value)
{
	System::Impl::ParameterList::add_parameter<T>(name, value);
}

/**
 * @brief Get the value of an existing parameter given its name.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @return T Current value of the Parameter.
 */
template <typename T> [[nodiscard]] T get_parameter(const char *name)
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
template <typename T> void set_parameter(const char *name, const T &value)
{
	System::Impl::ParameterList::set_parameter<T>(name, value);
}

template <typename T> void add_channel(const char *name, const T &value, const ChannelLogRate &log_rate)
{
	System::Impl::ChannelList::add_channel<T>(name, value, log_rate);
}

template <typename T> [[nodiscard]] T get_channel(const char *name)
{
	return System::Impl::ChannelList::get_channel<T>(name);
}

template <typename T> void set_channel(const char *name, const T &value)
{
    System::Impl::ChannelList::set_channel<T>(name, value);
}

} // namespace System

// clang-format off

#define INIT_MODULE(module_name)                                            \
namespace System                                                            \
{                                                                           \
namespace Generated                                                         \
{                                                                           \
static void init_##module_name(void);                                       \
static const uint32_t module_name##_tmp =                                   \
    System::Impl::ModuleManager::register_module_init(init_##module_name);  \
}                                                                           \
}                                                                           \
static void System::Generated::init_##module_name(void)


#define REGISTER_ROUTINE(name, frequency, stack_size)                                                                    \
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
 */                                                                                                            \
namespace System                                                                                                 \
{                                                                                                                \
namespace Generated                                                                                              \
{                                                                                                                \
static void name##_fn(void);                                                                                     \
static StackType_t name##_task_stack[stack_size];                                                                \
static StaticTask_t name##_task_storage;                                                                         \
static TaskHandle_t *name##_task_handle = NULL;                                                                        \
static StaticTimer_t name##_timer_storage;                                                                       \
                                                                                                                        \
static void name##_task_cb(void *parameters)                                                                     \
{                                                                                                                \
    while (true) {                                                                                           \
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);                                                   \
        name##_fn();                                                                                     \
    }                                                                                                        \
}                                                                                                                \
                                                                                                                        \
static void name##_timer_cb(TimerHandle_t xTimer)                                                                \
{                                                                                                                \
    if (name##_task_handle) { \
        xTaskNotify(*name##_task_handle, 0, eNoAction);                                                         \
    } \
}                                                                                                                \
                                                                                                                        \
static const int name##_tmp = System::Impl::RoutineManager::register_routine(                                    \
    #name, /* Routine name */                                                                                \
    frequency, /* Routine frequency */                                                                               \
    name##_task_cb, /* Routine task callback loop */                                                         \
    name##_task_stack, /* Routine task stack */                                                              \
    stack_size, /* Size of routine stack */                                                                  \
    &name##_task_storage, /* Routine name */                                                                 \
    &name##_task_handle, /* Routine task handle pointer pointer. Will be set by the register_routine call  */ \
    name##_timer_cb, /* Routine timer callback to trigger task */                                            \
    &name##_timer_storage /* Routine timer storage */                                                        \
);                                                                                                               \
                                                                                                                        \
} /* namespace Generated */                                                                                      \
} /* namespace System */                                                                                         \
                                                                                                                        \
static void System::Generated::name##_fn(void)

// clang-format on

#define GET_FN_PTR_OF_ROUTINE(name) System::Generated::name##_t::FunctionBody
