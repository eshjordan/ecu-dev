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

        if (connection->is_connected()) { connection->receive(); }

        vTaskDelay(1); // 10s timeout maybe?
    }

    vTaskDelete(nullptr);
}

void Connection::synchronize_connection(const uint64_t &num_messages)
{
    vLoggingPrintf("Synchronizing connection to %s, with %lld msgs...\n", ip_to_str(m_address.sin_addr), num_messages);

    BaseType_t tx_bytes;
    BaseType_t rx_bytes;

    Message_t recv_msg;
    Message_t send_msg;

    std::vector<Time_t> sent_times;
    sent_times.reserve(num_messages);
    std::vector<Time_t> received_times;
    received_times.reserve(num_messages);

    uint32_t id              = 0;
    Time_t stamp             = get_time_now();
    const char *acknowledge  = "sync_ack";
    uint64_t data            = 0;
    Message_t::Command_t cmd = Message_t::PING;
    make_message(&send_msg, id++, stamp, acknowledge, &data, cmd);

    // Send acknowledgement
    tx_bytes = FreeRTOS_send(m_socket, &send_msg, sizeof(Message_t), 0);

    if (tx_bytes < 0)
    {
        print_send_err(tx_bytes);
        disconnect();
        return;
    }

    int success_count = 0;
    int failed_count  = 0;
    while (success_count < num_messages)
    {
        rx_bytes = FreeRTOS_recv(m_socket, &recv_msg, sizeof(Message_t), 0);
        if (rx_bytes < 0)
        {
            print_recv_err(rx_bytes);
            disconnect();
            return;
        }

        Time_t recv_time = get_time_now();

        if (!looks_like_message(&recv_msg, rx_bytes)) { continue; }

        sent_times.push_back(recv_msg.header.stamp);
        received_times.push_back(recv_time);

        // Send acknowledgement
        stamp = get_time_now();
        make_message(&send_msg, id++, stamp, acknowledge, &data, cmd);

        tx_bytes = FreeRTOS_send(m_socket, &send_msg, sizeof(Message_t), 0);
        if (tx_bytes < 0)
        {
            print_send_err(tx_bytes);
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

    vLoggingPrintf("Time offset: %lld.%lld\n", seconds, nanoseconds);

    id                    = id + 1;
    stamp                 = get_time_now();
    const char *s_seconds = "seconds";
    data                  = seconds;
    cmd                   = Message_t::VALUE;
    make_message(&send_msg, id, stamp, s_seconds, &data, cmd);

    // Send seconds offset
    tx_bytes = FreeRTOS_send(m_socket, &send_msg, sizeof(Message_t), 0);
    if (tx_bytes < 0)
    {
        print_send_err(tx_bytes);
        disconnect();
        return;
    }

    id                        = id + 1;
    stamp                     = get_time_now();
    const char *s_nanoseconds = "nanoseconds";
    data                      = nanoseconds;
    cmd                       = Message_t::VALUE;
    make_message(&send_msg, id, stamp, s_nanoseconds, &data, cmd);

    // Send nanoseconds offset
    tx_bytes = FreeRTOS_send(m_socket, &send_msg, sizeof(Message_t), 0);
    if (tx_bytes < 0)
    {
        print_send_err(tx_bytes);
        disconnect();
        return;
    }
}

void Connection::receive()
{
    BaseType_t bytes_received = 0;

    bytes_received = FreeRTOS_recv(m_socket, m_rx_buffer, ipconfigNETWORK_MTU, 0);

    if (bytes_received > 0)
    {
        if (looks_like_message(m_rx_buffer, bytes_received))
        {
            Message_t msg;
            memcpy(&msg, m_rx_buffer, sizeof(Message_t));
            process_message(msg);
        } else
        {
            vLoggingPrintf("Processing as a command...\n");
            process_command((const char *)m_rx_buffer);
        }
    }
}

void Connection::process_message(const Message_t &message)
{
    switch (message.command)
    {
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

void Connection::process_command(const std::string &command)
{
    auto out = split(command, '/');

    std::cout << "Parameters:\n";
    for (auto &i : out)
    {
        std::cout << i << std::endl;
    }
    std::cout << "\n";

    if (out.size() > 0)
    {
        if (out[0] == "ecu")
        {
            if (out.size() > 1)
            {
                if (out[1] == "param")
                {
                    if (out.size() > 2)
                    {
                        auto name = out[2];
                        if (out.size() > 3)
                        {
                            if (out[3] == "get")
                            {
                                vLoggingPrintf("Parameter %s: %lf\n", name.c_str(),
                                               System::get_parameter<double>(name));
                            }
                            if (out[3] == "set" && out.size() > 4)
                            {
                                double value = std::stod(out[4]);
                                System::set_parameter(name, value);
                                vLoggingPrintf("Parameter %s set to %.2lf\n", name.c_str(), value);
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace Impl
} // namespace System
