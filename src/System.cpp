#include "System.hpp"

namespace System {

void init(void)
{
    if (!System::Impl::initialised)
    {

#ifdef ECU_ENABLE_NETWORKING
        /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
        are created in the vApplicationIPNetworkEventHook() hook function
        below.  The hook function is called when the network connects. */
        FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
#endif

        System::Impl::ParameterList::add_parameter("position", 0.0);
        System::Impl::ParameterList::add_parameter("velocity", 0.0);
        System::Impl::ParameterList::add_parameter("acceleration", 1.0);

        /* Start the RTOS schedule, set tasks and timers running. */
        vTaskStartScheduler();
    }

    System::Impl::initialised = true;
}

} // namespace System
