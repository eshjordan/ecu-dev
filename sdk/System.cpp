#include "System.hpp"
#include "FreeRTOS_IP.h"
#include "RTOS.hpp"
#include "projdefs.h"
#include <csignal>
#include <cstddef>
#include <cstring>
#include <string>
#include <unistd.h>

namespace System {

void run(void)
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, shutdown);

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

    /* Start the RTOS schedule, set tasks and timers running. */
    vTaskStartScheduler();
}

void shutdown(int signal)
{
    System::Impl::Server::shutdown();
    // /* Stop the RTOS schedule, stop tasks and timers running. */
    // vTaskEndScheduler();

    exit(0);
}

} // namespace System
