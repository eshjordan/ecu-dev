#pragma once

#include "ParameterList.hpp"
#include "RoutineManager.hpp"
#include "Server.hpp"

/**
 * @brief Hide the implementation of the system from the user. Expose only the simplest part of the interface.
 *
 */
namespace System {

namespace Impl {

/** @brief Stop chat users from resetting everything accidentally. */
static bool initialised = false;

/** @brief The routine manager. */
static bool network_up = false;

} // namespace Impl

/**
 * @brief Called upon program start. Set up parameters, etc. here, don't want users to add new ones in control loops.
 *
 */
void init(void);

/**
 * @brief 
 * 
 */
void run(void);

/**
 * @brief 
 * 
 */
void shutdown(void);

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
