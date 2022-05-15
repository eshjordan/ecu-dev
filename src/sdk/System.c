#include "System.h"
#include "CRC.h"
#include "RTOS.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int systemArgc = 0;

static const char systemArgv[64][64] = {0};

/** @brief Stop chat users from resetting everything accidentally. */
static boolean systemInitialised = FALSE;

/** @brief The routine manager. */
static boolean systemNetworkUp = FALSE;

extern void register_routines(void);

static void end(int signal)
{
    systemShutdown(signal);
    exit(0);
}


const char* systemGetExecutablePath() {
    (void)sizeof(systemArgv);
    return systemArgv[0];
}

void systemInit(int argc, char **argv)
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, end);

    if (!systemInitialised)
    {
        systemArgc = argc;
        for (int i = 0; i < argc; i++)
        {
            memcpy(systemArgv[i], argv[i], strlen(argv[i]));
        }

        init_crc();
#ifndef STM32_BUILD
        serverInit();
#endif
        
        port_init_io();

        double pos = 0.0;
        double vel = 0.0;
        double acc = 1.0;

        parameterListAddParameter("position", &pos, PARAM_DOUBLE, sizeof(double));
        parameterListAddParameter("velocity", &vel, PARAM_DOUBLE, sizeof(double));
        parameterListAddParameter("acceleration", &acc, PARAM_DOUBLE, sizeof(double));

        register_routines();
    }

    systemInitialised = TRUE;
}

void systemRun(void)
{
    /* Start the RTOS schedule, set tasks and timers running. */
    vTaskStartScheduler();
}

void systemRestart(int signal)
{
    systemShutdown(signal);

    printf("Shutdown ok\n");

    /* Restart the program */
    printf("Restarting - %s\n", systemArgv[0]);

    // Currently the child becomes an orphan. This is apparently ok? as it gets adopted by init at the highest level.
    boolean parent = FALSE;
    boolean child  = FALSE;

    pid_t pid_1 = fork();

    parent = pid_1 != 0;
    child  = pid_1 == 0;

    if (child)
    {
        setsid();
        execv(systemArgv[0], systemArgv);
    }

    if (child) { exit(0); }
}

void systemShutdown(int signal)
{
#ifndef STM32_BUILD
    serverShutdown();
#endif

    // /* Stop the RTOS schedule, stop tasks and timers running. */
    // vTaskEndScheduler();
}


uint32_t systemGetParameter(const char *const name, void *const value, ParameterType_en *const type)
{
	return parameterListGetParameter(name, value, type);
}

void systemSetParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size)
{
	parameterListSetParameter(name, value, type, size);
}
