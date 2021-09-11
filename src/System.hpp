#pragma once

#include "ParameterList.hpp"
#include "RoutineManager.hpp"

namespace System {

void init(void);

template <typename T> [[nodiscard]] T get_parameter(const std::string &name)
{
    return System::Impl::ParameterList::get_parameter<T>(name);
}

template <typename T> void set_parameter(const std::string &name, const T &value)
{
    System::Impl::ParameterList::set_parameter<T>(name, value);
}

} // namespace System
