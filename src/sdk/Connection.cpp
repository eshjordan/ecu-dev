#ifndef STM32_BUILD

#include "Connection.hpp"
#include "CRC.h"
#include "ECU_Msg.h"
#include "RTOS_IP.hpp"
#include "System.hpp"
#include <cstddef>
#include <cstdint>

namespace System {
namespace Impl {

Connection::Connection(const Socket_t &socket, const freertos_sockaddr &address) : m_socket(socket), m_address(address)
{
    init();

    xTaskCreate(manage_connection, "connection_task", 10000, /* Stack size in words, not bytes. */
                (void *)this,                                /* Parameter passed into the task. */
                tskIDLE_PRIORITY,                            /* Priority of the task. */
                const_cast<TaskHandle_t *>(&m_run_task));    /* Task handle. */
}

void Connection::init(void)
{
    ip_to_str(const_cast<char *>(m_ip_str), m_address.sin_addr);
    m_connected = m_open = FreeRTOS_issocketconnected(m_socket);
    vLoggingPrintf("Connection opened! - %s\n", m_ip_str);
}

BaseType_t Connection::receive_message(ECU_Msg_t *message)
{
    BaseType_t rx_bytes = FreeRTOS_recv(m_socket, message, sizeof(ECU_Msg_t), 0);
    if (rx_bytes < 0)
    {
        print_recv_err(rx_bytes);
        if (rx_bytes == -pdFREERTOS_ERRNO_ENOTCONN) { close(); }
    }
    return rx_bytes;
}

BaseType_t Connection::send_message(ECU_Msg_t *message)
{
    BaseType_t tx_bytes = FreeRTOS_send(m_socket, message, sizeof(ECU_Msg_t), 0);
    if (tx_bytes < 0)
    {
        print_send_err(tx_bytes);
        if (tx_bytes == -pdFREERTOS_ERRNO_ENOTCONN) { close(); }
    }
    return tx_bytes;
}

void Connection::disconnect(void)
{
    if (!is_open())
    {
        vLoggingPrintf("Already disconnected! - %s\n", m_ip_str);
        return;
    }

    m_open = false;

    FreeRTOS_shutdown(m_socket, FREERTOS_SHUT_RDWR);

    vLoggingPrintf("Disconnected! - %s\n", m_ip_str);
}

void Connection::close(void)
{
    disconnect();

    if (!is_connected())
    {
        vLoggingPrintf("Already closed! - %s\n", m_ip_str);
        return;
    }

    m_connected = false;

    FreeRTOS_closesocket(m_socket);

    vLoggingPrintf("Connection closed! - %s\n", m_ip_str);

    // vTaskDelete(m_run_task);
}

bool Connection::is_connected(void) { return m_connected; }

bool Connection::is_open(void) { return m_open; }

void Connection::manage_connection(void *arg)
{
    /* This should be the only place we have a non-shared ptr to the connection, so we can use it to delete the task
     * properly */
    auto *connection = (Connection *)arg;

    while (connection->is_open())
    {
        if (connection->is_open() && connection->is_connected())
        {
            ECU_Msg_t msg = {};
            if (connection->receive_message(&msg) <= 0) { continue; }

            int message_ok = ecu_msg_check(&msg);
            if (message_ok < 0)
            {
                char err_msg[32];
                ecu_err_to_str(err_msg, message_ok);
                printf("%s\n", err_msg);
                continue;
            }

            connection->process_message(&msg);
        }

        if (connection->is_open()) { vTaskDelay(1); }
    }

    vTaskDelete(nullptr);
}

void Connection::process_message(ECU_Msg_t *message)
{
    printf("Received Message:\nName: %s\nID: %u\nCommand: %u\nData: %lld\n", message->name, message->header.id,
           message->command, (const uint64_t &)message->data);

    switch (message->command)
    {
    case ECU_Msg_t::UNKNOWN_CMD: {
        break;
    }
    case ECU_Msg_t::ECHO_CMD: {
        send_message(message);
        break;
    }
    case ECU_Msg_t::PING_CMD: {
        send_message(message);
        break;
    }
    case ECU_Msg_t::ACK_CMD: {
        break;
    }
    case ECU_Msg_t::RESTART_CMD: {
        // Shut down all connections nicely
        xTaskCreate(System::restart, "ECU_RESTART", 10000, /* Stack size in words, not bytes. */
                    memset(pvPortMalloc(1), 0, 1),    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                      /* Priority of the task. */
                    nullptr);                              /* Don't need to keep the task handle. */

        // Terminate the connection from our end. Task will still be running, but we can't send or recv messages.
        close();
        break;
    }
    case ECU_Msg_t::STATUS_CMD: {
        send_status(message);
        break;
    }
    case ECU_Msg_t::SYNC_CMD: {
        synchronize_connection(message);
        break;
    }
    case ECU_Msg_t::FIRMWARE_UPDATE_CMD: {
        download_firmware(message);

        // // Shut down all connections nicely
        // xTaskCreate(System::restart, "ECU_RESTART", 10000, /* Stack size in words, not bytes. */
        //             memset(malloc(1), EXIT_SUCCESS, 1),    /* Parameter passed into the task. */
        //             tskIDLE_PRIORITY,                      /* Priority of the task. */
        //             nullptr);                              /* Don't need to keep the task handle. */

        // // Terminate the connection from our end. Task will still be running, but we can't send or recv messages.
        // close();
        break;
    }
    case ECU_Msg_t::PROGRAM_UPDATE_CMD: {
        download_firmware(message);

        // Shut down all connections nicely
        xTaskCreate(System::restart, "ECU_RESTART", 10000, /* Stack size in words, not bytes. */
                    memset(pvPortMalloc(1), 0, 1),    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,                      /* Priority of the task. */
                    nullptr);                              /* Don't need to keep the task handle. */

        // Terminate the connection from our end. Task will still be running, but we can't send or recv messages.
        close();

        break;
    }
    case ECU_Msg_t::VALUE_CMD: {
        break;
    }
    case ECU_Msg_t::PARAM_GET_CMD: {
        break;
    }
    case ECU_Msg_t::PARAM_SET_CMD: {
        break;
    }
    }
}

} // namespace Impl
} // namespace System

#endif
