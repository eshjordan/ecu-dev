/* System includes. */
#include "System.hpp"
#include <cstdio>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifdef STM32_BUILD
#include "mock_rtos.c"
#endif


int main(int argc, char *argv[])
{
    System::init(argc, argv);

    /* Add tasks here. */
    System::run();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */

    return 0;
}
