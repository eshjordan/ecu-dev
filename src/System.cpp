#include "System.hpp"

namespace System {

void init(void)
{
    System::Impl::ParameterList::add_parameter("position", 0.0);
    System::Impl::ParameterList::add_parameter("velocity", 0.0);
    System::Impl::ParameterList::add_parameter("acceleration", 1.0);
}

} // namespace System
