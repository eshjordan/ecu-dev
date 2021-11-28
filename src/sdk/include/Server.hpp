#ifndef STM32_BUILD

#pragma once

#include "Connection.hpp"
#include <memory>
#include <vector>

namespace System {
namespace Impl {

class Server
{
private:
    static bool server_started;

    static Socket_t server_socket;

    static TaskHandle_t listen_task_handle;

    static std::vector<std::shared_ptr<Connection>> connections;

    static void start_task(void *arg);

    static void listen_task(void *arg);

public:
    static void init(void);

    static void start(void);

    static void shutdown(void);
};

} // namespace Impl
} // namespace System

#endif
