#ifndef STM32_BUILD

#pragma once

#include "ECU_Msg.h"
#include "RTOS_IP.hpp"

namespace System {
namespace Impl {

class Connection
{

private:
    const Socket_t m_socket                      = {};
    const freertos_sockaddr m_address            = {};
    const char m_ip_str[16]                      = {0};
    const struct tskTaskControlBlock *m_run_task = {};
    uint32_t m_seq                               = {0};
    bool m_connected                             = false;
    bool m_open                                  = false;
    uint8_t m_rx_buffer[ipconfigNETWORK_MTU]     = {0};
    uint8_t m_tx_buffer[ipconfigNETWORK_MTU]     = {0};

    void init(void);

    static void manage_connection(void *arg);

    void process_message(ECU_Msg_t *message);

    BaseType_t receive_message(ECU_Msg_t *message);

    BaseType_t send_message(ECU_Msg_t *message);

    /* Functions to handle specific requests. */

    void send_status(ECU_Msg_t *message);

    void synchronize_connection(ECU_Msg_t *num_messages);

    void download_firmware(ECU_Msg_t *message);

public:
    Connection(const Socket_t &socket, const freertos_sockaddr &address);

    ~Connection(void) { close(); }

    void disconnect(void);

    void close(void);

    bool is_connected(void);

    bool is_open(void);
};

} // namespace Impl
} // namespace System

#endif
