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
        /* Initialise the RTOS's TCP/IP stack. The tasks that use the network are created in the
         * vApplicationIPNetworkEventHook() hook function below. The hook function is called when the network connects.
         */

        BaseType_t init_result =
            FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);

        // if (init_result != pdPASS) { throw std::runtime_error("FreeRTOS+TCP Failed to initialise!"); }

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
