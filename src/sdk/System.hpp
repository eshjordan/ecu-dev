#pragma once

#include "ParameterList.hpp"
#include "RoutineManager.hpp"

/**
 * @brief Hide the implementation of the system from the user. Expose only the simplest part of the interface.
 *
 */
namespace System {

namespace Impl {

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
