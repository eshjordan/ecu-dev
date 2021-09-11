/* Standard includes. */
#include <csignal>

#include "RTOS.hpp"

/* System includes. */
#include "System.hpp"

/**
 * @brief Signal handler for Ctrl_C to cause the program to exit.
 *
 * @param signal
 */
static void handle_sigint(int signal) { exit(signal); }

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, handle_sigint);

    /* Add tasks here. */
    System::init();

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */

    return 0;
}
