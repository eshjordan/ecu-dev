#pragma once

#include "Message.h"
#include "RTOS.hpp"
#include "RTOS_IP.hpp"
#include <string>

namespace System {
namespace Impl {

class Server
{
private:
    static bool server_started;

    static Socket_t client_socket;

    static TaskHandle_t listen_task_handle;

    static void start_task(void *arg);

    static void listen_task(void *arg);

    static void connection_task(void *arg);

    static void process_message(const Message_t &message);

    static void process_command(const std::string &command);

public:
    static void init(void);

    static void start(void);

    static void shutdown(void);
};

} // namespace Impl
} // namespace System
