#include "Connection.hpp"
#include "Message.h"
#include "RTOS_IP.hpp"
#include "System.hpp"
#include "portmacro.h"
#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

namespace System {
namespace Impl {

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

void Connection::synchronize_connection(const uint64_t &num_messages)
{
    vLoggingPrintf("Synchronizing connection to %s, with %lld msgs...\n", ip_to_str(m_address.sin_addr), num_messages);

    std::vector<Time_t> sent_times;
    sent_times.reserve(num_messages);
    std::vector<Time_t> received_times;
    received_times.reserve(num_messages);

    Message_t ack_msg;
    make_message(&ack_msg,      /* Destination message. */
                 m_seq++,       /* Message ID number. */
                 "sync_ack",    /* Message name. */
                 nullptr,       /* Message data, set to zero. */
                 Message_t::ACK /* Command. */
    );

    // Send acknowledgement
    if (send_message(ack_msg) < 0)
    {
        disconnect();
        return;
    }

    int success_count = 0;
    int failed_count  = 0;
    while (success_count < num_messages)
    {
        if (failed_count > 10)
        {
            vLoggingPrintf("Failed to receive %d messages, disconnecting...\n", failed_count);
            disconnect();
            return;
        }

        Message_t recv_msg;
        BaseType_t rx_bytes = receive_message(&recv_msg);
        if (rx_bytes < 0)
        {
            disconnect();
            return;
        }

        Time_t recv_time = get_time_now();

        int msg_status = check_message(&recv_msg);
        if (msg_status < 0)
        {
            print_msg_err(msg_status);
            failed_count++;
            continue;
        }

        switch (recv_msg.command)
        {
        case Message_t::PING:
        case Message_t::ACK:
        case Message_t::SYNC: {
            break;
        }
        default: {
            puts("Bad message command, must send one of: PING, ACK, SYNC");
            failed_count++;
            continue;
        }
        }

        sent_times.push_back(recv_msg.header.stamp);
        received_times.push_back(recv_time);

        // Send acknowledgement
        Message_t ack_msg;
        make_message(&ack_msg,      /* Destination message. */
                     m_seq++,       /* Message ID number. */
                     "sync_ack",    /* Message name. */
                     nullptr,       /* Message data, set to zero. */
                     Message_t::ACK /* Command. */
        );

        BaseType_t tx_bytes = send_message(ack_msg);
        if (tx_bytes < 0)
        {
            disconnect();
            return;
        }

        success_count++;
    }

    int64_t ns_total = 0;

    // Send time offset
    for (int i = 0; i < num_messages; i++)
    {
        int64_t seconds     = received_times[i].tv_sec - sent_times[i].tv_sec;
        int64_t nanoseconds = received_times[i].tv_nsec - sent_times[i].tv_nsec;

        if (seconds > 0 && nanoseconds < 0)
        {
            nanoseconds += 1000000000;
            seconds--;
        } else if (seconds < 0 && nanoseconds > 0)
        {
            nanoseconds -= 1000000000;
            seconds++;
        }

        ns_total += seconds * 1000000000 + nanoseconds;
    }

    // Average time
    int64_t avg_time    = ns_total / (int64_t)num_messages;
    int64_t seconds     = avg_time / 1000000000;
    int64_t nanoseconds = avg_time % 1000000000;

    vLoggingPrintf("Time offset - %s: %lld.%lld sec\n", ip_to_str(m_address.sin_addr), seconds, nanoseconds);

    Message_t seconds_msg;
    make_message(&seconds_msg,    /* Destination message. */
                 m_seq++,         /* Message ID number. */
                 "seconds",       /* Message name. */
                 &seconds,        /* Message data, set to zero. */
                 Message_t::VALUE /* Command. */
    );

    // Send seconds offset
    BaseType_t tx_bytes = send_message(seconds_msg);
    if (tx_bytes < 0)
    {
        disconnect();
        return;
    }

    Message_t nanoseconds_msg;
    make_message(&nanoseconds_msg, /* Destination message. */
                 m_seq++,          /* Message ID number. */
                 "nanoseconds",    /* Message name. */
                 &nanoseconds,     /* Message data, set to zero. */
                 Message_t::VALUE  /* Command. */
    );

    // Send nanoseconds offset
    tx_bytes = send_message(nanoseconds_msg);
    if (tx_bytes < 0)
    {
        disconnect();
        return;
    }
}

BaseType_t Connection::receive_message(const Message_t *message)
{
    BaseType_t rx_bytes = FreeRTOS_recv(m_socket, (void *)message, sizeof(Message_t), 0);
    if (rx_bytes < 0) { print_recv_err(rx_bytes); }
    return rx_bytes;
}

BaseType_t Connection::send_message(const Message_t &message)
{
    BaseType_t tx_bytes = FreeRTOS_send(m_socket, &message, sizeof(Message_t), 0);
    if (tx_bytes < 0) { print_send_err(tx_bytes); }
    return tx_bytes;
}

void Connection::process_message(const Message_t &message)
{
    switch (message.command)
    {
    case Message_t::ECHO: {
        send_message(message);
        break;
    }
    case Message_t::SYNC: {
        synchronize_connection((const uint64_t &)message.data);
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
