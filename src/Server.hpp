#pragma once

#include "RTOS.hpp"
#include <stdexcept>

namespace System {
namespace Impl {

class Server
{
private:
    static void start_task(void *arg);

    static bool server_started;

    static Socket_t server_socket;

public:
    static void init(void);

    static void start(void);
};

} // namespace Impl
} // namespace System
