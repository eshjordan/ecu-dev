#include "System.hpp"
#include "CRC.h"
#include "RTOS.hpp"
#include <csignal>
#include <cstring>
#include <iostream>
#include <unistd.h>

static void end(int signal)
{
    System::shutdown(signal);
    exit(0);
}

namespace System {

namespace Impl {

class Server
{
public:
    static void init(void);

    static void start(void);

    static void shutdown(void);
};

} // namespace Impl

std::string System::Impl::get_executable_path() { return System::Impl::s_argv[0]; }

void init(int argc, char **argv)
{
    /* SIGINT is not blocked by the posix port */
    signal(SIGINT, end);

    if (!System::Impl::initialised)
    {
        System::Impl::s_argc = argc;
        for (int i = 0; i < argc; i++)
        {
            System::Impl::s_argv.emplace_back(argv[i]);
        }

        init_crc();
        System::Impl::Server::init();

        System::Impl::ParameterList::add_parameter("position", 0.0);
        System::Impl::ParameterList::add_parameter("velocity", 0.0);
        System::Impl::ParameterList::add_parameter("acceleration", 1.0);
    }

    System::Impl::initialised = true;
}

void run(void)
{
    /* Start the RTOS schedule, set tasks and timers running. */
    vTaskStartScheduler();
}

void restart(int signal)
{
    System::shutdown(signal);

    puts("Shutdown ok");

    /* Restart the program */

    char **argv = new char *[System::Impl::s_argc + 1];
    for (int i = 0; i < System::Impl::s_argc; i++)
    {
        argv[i] = new char[System::Impl::s_argv[i].size() + 1];
        strcpy(argv[i], System::Impl::s_argv[i].c_str());
        std::cout << argv[i] << std::endl;
    }
    argv[System::Impl::s_argc] = nullptr;

    std::cout << "Restarting - " << argv[0] << std::endl;

    // Currently the child becomes an orphan. This is apparently ok? as it gets adopted by init at the highest level.
    bool parent = false;
    bool child  = false;

    pid_t pid_1 = fork();

    parent = pid_1 != 0;
    child  = pid_1 == 0;

    if (child)
    {
        setsid();
        execv(argv[0], argv);
    }

    if (child) { exit(0); }
}

void shutdown(int signal)
{
    System::Impl::Server::shutdown();

    // /* Stop the RTOS schedule, stop tasks and timers running. */
    // vTaskEndScheduler();
}

} // namespace System
