#pragma once

#include "RTOS.hpp"
#include <stdexcept>

namespace System {
namespace Impl {

class Server
{
private:
    static bool server_started;

    static Socket_t server_socket;

    static TaskHandle_t listen_task_handle;

    static void start_task(void *arg);

    static void listen_task(void *arg);

    static void connection_task(void *arg);

public:
    static void init(void);

    static void start(void);
};

} // namespace Impl
} // namespace System
