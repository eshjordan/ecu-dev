#include "System.hpp"
#include <csignal>

#ifdef ECU_ENABLE_NETWORKING
#include "CRC.h"
#include "FreeRTOS_IP.h"
#include "Server.hpp"
#endif

namespace System {

void run(void)
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, shutdown);

    if (!System::Impl::initialised)
    {

#ifdef ECU_ENABLE_NETWORKING

        init_crc();
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
#ifdef ECU_ENABLE_NETWORKING
    System::Impl::Server::shutdown();
#endif
    // /* Stop the RTOS schedule, stop tasks and timers running. */
    // vTaskEndScheduler();

    exit(0);
}

} // namespace System
