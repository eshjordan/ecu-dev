/* Standard includes. */
#include <csignal>
#include <iostream>

#include "RTOS.hpp"

/* Program includes. */
#include "Routine.hpp"
#include "System.hpp"

/**
 * @brief Signal handler for Ctrl_C to cause the program to exit.
 *
 * @param signal
 */
static void handle_sigint(int signal) { exit(signal); }

/**
 * @brief Test routine to print to the console.
 *
 */
REGISTER_ROUTINE(test, 1) { std::cout << "hello world!\n"; }

int main(int /*argc*/, char * /*argv*/[])
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, handle_sigint);

    // init_application();

    /* Add tasks here. */

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */

    return 0;
}
