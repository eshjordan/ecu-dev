#pragma once

#include "types.h"
#include "ParameterList.h"
#include "IO.h"
#include "RoutineManager.h"


/**
 * @brief Get the executable path object
 *
 * @return const char * Path to the executable, from argv[0]
 */
const char *systemGetExecutablePath();

/**
 * @brief
 *
 */
void systemInit(int argc, char **argv);

/**
 * @brief
 *
 */
void systemRun(void);

/**
 * @brief
 *
 */
void systemRestart(int signal);


/**
 * @brief
 *
 */
void systemShutdown(int signal);

/**
 * @brief Get the value of an existing parameter given its name.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @return T Current value of the Parameter.
 */
uint32_t systemGetParameter(const char *const name, void *const value, ParameterType_en *const type);

/**
 * @brief Set the value of an existing parameter given its name.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @param value New value of the Parameter.
 */
void systemSetParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size);

// clang-format off

#define DECLARE_ROUTINE(name, frequency, stack_size)                                                                    \
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
                                                                                                              \
extern void name##_fn(void);                                                                                     \
extern StackType_t name##_task_stack[stack_size];                                                                \
extern StaticTask_t name##_task_storage;                                                                         \
extern TaskHandle_t *name##_task_handle;                                                                        \
extern StaticTimer_t name##_timer_storage;                                                                       \
                                                                                                                        \
extern void name##_task_cb(void *parameters);                                                                                                          \
                                                                                                                        \
extern void name##_timer_cb(TimerHandle_t xTimer);                                                                \


#define DEFINE_ROUTINE(name, frequency, stack_size)                                                                    \
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
                                                                                                              \
void name##_fn(void);                                                                                     \
StackType_t name##_task_stack[stack_size] = {0};                                                                \
StaticTask_t name##_task_storage;                                                                         \
TaskHandle_t *name##_task_handle = NULL;                                                                        \
StaticTimer_t name##_timer_storage;                                                                       \
                                                                                                                        \
void name##_task_cb(void *parameters)                                                                     \
{                                                                                                                \
    while (TRUE) {                                                                                           \
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);                                                   \
        name##_fn();                                                                                     \
    }                                                                                                        \
}                                                                                                                \
                                                                                                                        \
void name##_timer_cb(TimerHandle_t xTimer)                                                                \
{                                                                                                                \
    if (name##_task_handle) { \
        xTaskNotify(*name##_task_handle, 0, eNoAction);                                                         \
    } \
}                                                                                                                \
                                                                                                                        \
void name##_fn(void)


#define REGISTER_ROUTINE(name, frequency, stack_size)                                                                    \
register_routine(                                    \
    #name, /* Routine name */                                                                                \
    frequency, /* Routine frequency */                                                                               \
    name##_task_cb, /* Routine task callback loop */                                                         \
    name##_task_stack, /* Routine task stack */                                                              \
    stack_size, /* Size of routine stack */                                                                  \
    &name##_task_storage, /* Routine name */                                                                 \
    &name##_task_handle, /* Routine task handle pointer pointer. Will be set by the register_routine call  */ \
    name##_timer_cb, /* Routine timer callback to trigger task */                                            \
    &name##_timer_storage /* Routine timer storage */                                                        \
);


// clang-format on
