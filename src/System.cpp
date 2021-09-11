#include "System.hpp"

namespace System {

void init(void)
{
    if (!System::Impl::initialised)
    {
        System::Impl::ParameterList::add_parameter("position", 0.0);
        System::Impl::ParameterList::add_parameter("velocity", 0.0);
        System::Impl::ParameterList::add_parameter("acceleration", 1.0);
    }
    System::Impl::initialised = true;
}

} // namespace System
