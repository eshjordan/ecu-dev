#pragma once

#include "Message.h"
#include "RTOS_IP.hpp"
#include "portmacro.h"
#include <string>

namespace System {
namespace Impl {

class Connection
{

private:
    const Socket_t m_socket{};
    const freertos_sockaddr m_address{};
    uint32_t m_seq{0};
    TaskHandle_t m_run_task{};
    bool m_destroy                           = false;
    uint8_t m_rx_buffer[ipconfigNETWORK_MTU] = {0};
    uint8_t m_tx_buffer[ipconfigNETWORK_MTU] = {0};

    inline void init(void) const { vLoggingPrintf("Connection opened! - %s\n", ip_to_str(m_address.sin_addr)); }

    static void manage_connection(void *arg);

    BaseType_t receive_message(const Message_t *message);

    BaseType_t send_message(const Message_t &message);

    void process_message(const Message_t &message);

    void echo_message(const Message_t &message);

    void synchronize_connection(const uint64_t &num_messages);

public:
    Connection(const Socket_t &socket, const freertos_sockaddr &address) : m_socket(socket), m_address(address)
    {
        init();

        xTaskCreate(manage_connection, "connection_task", 10000, /* Stack size in words, not bytes. */
                    (void *)this,                                /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                            /* Priority of the task. */
                    &m_run_task);                                /* Task handle. */
    }

    ~Connection(void)
    {
        if (is_connected()) { disconnect(); }

        vLoggingPrintf("Connection closed! - %s\n", ip_to_str(m_address.sin_addr));

        m_destroy = true;
    }

    inline void disconnect(void)
    {
        FreeRTOS_shutdown(m_socket, FREERTOS_SHUT_RDWR);
        FreeRTOS_closesocket(m_socket);
        vLoggingPrintf("Disconnected! - %s\n", ip_to_str(m_address.sin_addr));
    }

    inline bool is_connected(void) { return FreeRTOS_issocketconnected(m_socket); }
};

} // namespace Impl
} // namespace System
