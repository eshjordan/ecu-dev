#include "Connection.hpp"

namespace System {
namespace Impl {

Connection::Connection(const Socket_t &socket, const freertos_sockaddr &address) : m_socket(socket), m_address(address)
{
    init();

    xTaskCreate(manage_connection, "connection_task", 10000, /* Stack size in words, not bytes. */
                (void *)this,                                /* Parameter passed into the task. */
                tskIDLE_PRIORITY,                            /* Priority of the task. */
                &m_run_task);                                /* Task handle. */
}

void Connection::manage_connection(void *arg)
{
    /* This should be the only place we have a non-shared ptr to the connection, so we can use it to delete the task
     * properly */
    auto *connection = (Connection *)arg;

    while (true)
    {
        if (connection->m_destroy || connection == nullptr)
        {
            vTaskDelete(nullptr);
            break;
        }

        if (connection->is_connected())
        {
            Message_t msg;
            if (connection->receive_message(&msg) < 0) { continue; }

            int message_ok = check_message(&msg);
            if (message_ok < 0)
            {
                print_msg_err(message_ok);
                continue;
            }

            connection->process_message(msg);
        }
    }

    vTaskDelete(nullptr);
}

void Connection::process_message(const Message_t &message)
{
    switch (message.command)
    {
    case Message_t::ECHO:
    case Message_t::PING: {
        send_message(&message);
        break;
    }
    case Message_t::SYNC: {
        synchronize_connection(message);
        break;
    }
    case Message_t::FIRMWARE_UPDATE: {
        update_firmware(message);
        break;
    }
    default: {
        vLoggingPrintf("Command not implemented yet!\n");
        break;
    }
    }

    printf("Received Message:\nName: %s\nID: %u\nCommand: %u\nData: %lld\n", message.name, message.header.id,
           message.command, (const uint64_t &)message.data);
}

} // namespace Impl
} // namespace System
