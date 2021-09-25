/* System includes. */
#include "System.hpp"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* Add tasks here. */
    System::run();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */

    return 0;
}
