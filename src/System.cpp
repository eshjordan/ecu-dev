#include "System.hpp"
#include "FreeRTOS_IP.h"
#include "RTOS.hpp"
#include "projdefs.h"
#include <cstddef>
#include <cstring>
#include <string>
#include <unistd.h>

namespace System {

void init(void)
{
    if (!System::Impl::initialised)
    {

#ifdef ECU_ENABLE_NETWORKING
        
        System::Impl::Server::init();

#endif

        System::Impl::ParameterList::add_parameter("position", 0.0);
        System::Impl::ParameterList::add_parameter("velocity", 0.0);
        System::Impl::ParameterList::add_parameter("acceleration", 1.0);
    }

    System::Impl::initialised = true;
}

void run(void)
{
    /* Start the RTOS schedule, set tasks and timers running. */
    vTaskStartScheduler();
}

} // namespace System
