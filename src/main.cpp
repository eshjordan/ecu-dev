/* Standard includes. */
#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <utility>

#include "RTOS.hpp"

/* Program includes. */
#include "Routine.hpp"
#include "System.hpp"

TickType_t a[1000];
int c = 0;

/*
 * The callback function executed when the software timer expires.
 */
void timer_callback(TimerHandle_t xTimerHandle)
{
    System sys = System::get_instance();
    sys.run_routines();
    console_print("timer call");
}

void cb(void *params)
{
    TickType_t prev_time = xTaskGetTickCount();
    a[c++]               = prev_time;
    while (true)
    {
        TickType_t curr_time = xTaskGetTickCount();
        a[c]                 = curr_time - prev_time;
        std::cout << curr_time << ", " << a[c] << std::endl;
        System sys = System::get_instance();
        sys.run_routines();
        prev_time = curr_time;
        c++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Signal handler for Ctrl_C to cause the program to exit, and generate the profiling info.
 *
 * @param signal
 */
static void handle_sigint(int /*signal*/)
{
    int x_return = 0;

    x_return = chdir(BUILD_DIR); // changing dir to place gmon.out inside build

    if (x_return == -1) { printf("chdir into %s error is %d\n", BUILD_DIR, errno); }

    exit(2);
}

REGISTER_ROUTINE(test, void *parameters)
{
    // printf("hello world!\n");
    while (true)
    {
        std::cout << "hello world!\n";
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(int /*argc*/, char * /*argv*/[])
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, handle_sigint);

    init_application();

    /* Run stuff here */

    /* A software timer that is started from the tick hook. */
    TimerHandle_t x_timer = nullptr;

    TaskHandle_t *tsk_handle = nullptr;

    /* Start the two tasks as described in the comments at the top of this file. */
    xTaskCreate(cb,          /* The function that implements the task. */
                "mytask",    /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                1000,        /* The size of the stack to allocate to the task. */
                nullptr,     /* The parameter passed to the task - not used in this simple case. */
                1,           /* The priority assigned to the task. */
                tsk_handle); /* The task handle is not required, so NULL is passed. */

    /* Create the software timer, but don't start it yet. */
    // x_timer = xTimerCreate(
    //     "Timer", /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
    //     pdMS_TO_TICKS(2000), /* The period of the software timer in ticks. */
    //     true,                /* xAutoReload is set to pdTRUE. */
    //     nullptr,             /* The timer's ID is not used. */
    //     timer_callback);     /* The function executed when the timer expires. */

    // if (x_timer != nullptr) { xTimerStart(x_timer, 0); }

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    // while (1) {}

    return 0;
}
