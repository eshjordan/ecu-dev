#pragma once

#include "Message.h"
#include "RTOS_IP.hpp"

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

    void process_message(Message_t *message);

    inline BaseType_t receive_message(Message_t *message)
    {
        BaseType_t rx_bytes = FreeRTOS_recv(m_socket, message, sizeof(Message_t), 0);
        if (rx_bytes < 0) { print_recv_err(rx_bytes); }
        return rx_bytes;
    }

    inline BaseType_t send_message(Message_t *message)
    {
        BaseType_t tx_bytes = FreeRTOS_send(m_socket, message, sizeof(Message_t), 0);
        if (tx_bytes < 0) { print_send_err(tx_bytes); }
        return tx_bytes;
    }

    /* Functions to handle specific requests. */

    void synchronize_connection(Message_t *num_messages);

    void update_firmware(Message_t *message);

public:
    Connection(const Socket_t &socket, const freertos_sockaddr &address);

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
