#include "Connection.hpp"
#include "System.hpp"
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace System {
namespace Impl {

void Connection::synchronize_connection(Message_t *message)
{
    auto num_messages = *(uint64_t *)&message->data;

    vLoggingPrintf("Synchronizing connection to %s, with %lld msgs...\n", ip_to_str(m_address->sin_addr), num_messages);

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
    if (send_message(&ack_msg) < 0)
    {
        close();
        return;
    }

    int success_count = 0;
    int failed_count  = 0;
    while (success_count < num_messages)
    {
        if (failed_count > 10)
        {
            vLoggingPrintf("Failed to receive %d messages, closing connection...\n", failed_count);
            close();
            return;
        }

        Message_t recv_msg;
        BaseType_t rx_bytes = receive_message(&recv_msg);
        if (rx_bytes < 0)
        {
            close();
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

        BaseType_t tx_bytes = send_message(&ack_msg);
        if (tx_bytes < 0)
        {
            close();
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

    vLoggingPrintf("Time offset - %s: %lld.%lld sec\n", ip_to_str(m_address->sin_addr), seconds, nanoseconds);

    Message_t seconds_msg;
    make_message(&seconds_msg,    /* Destination message. */
                 m_seq++,         /* Message ID number. */
                 "seconds",       /* Message name. */
                 &seconds,        /* Message data, set to zero. */
                 Message_t::VALUE /* Command. */
    );

    // Send seconds offset
    BaseType_t tx_bytes = send_message(&seconds_msg);
    if (tx_bytes < 0)
    {
        close();
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
    tx_bytes = send_message(&nanoseconds_msg);
    if (tx_bytes < 0)
    {
        close();
        return;
    }
}

void Connection::download_firmware(Message_t *message)
{
    auto firmware_size = ((uint32_t *)&message->data)[0];
    auto firmware_crc  = ((uint32_t *)&message->data)[1];

    printf("Firmware size: %u\n", firmware_size);
    printf("Firmware CRC: %u\n", firmware_crc);

    Message_t ack_msg;
    make_message(&ack_msg,      /* Destination message. */
                 m_seq++,       /* Message ID number. */
                 "sync_ack",    /* Message name. */
                 nullptr,       /* Message data, set to zero. */
                 Message_t::ACK /* Command. */
    );

    // Send acknowledgement
    if (send_message(&ack_msg) < 0)
    {
        close();
        return;
    }

    auto *received_firmware = (uint8_t *)pvPortMalloc(firmware_size);
    memset(received_firmware, 0, firmware_size);

    BaseType_t success_count = 0;
    int failed_count         = 0;
    int i                    = 0;
    while (success_count < firmware_size)
    {
        memset(m_rx_buffer, 0, sizeof(m_rx_buffer));
        BaseType_t rx_bytes = FreeRTOS_recv(*m_socket, received_firmware + success_count, ipconfigTCP_MSS, 0);

        if (rx_bytes < 0)
        {
            print_recv_err(rx_bytes);
            vPortFree(received_firmware);
            close();
            return;
        }

        success_count += rx_bytes;

        i++;
    }

    printf("Received %ld bytes\n", success_count);

    // Check CRC
    CRC transmitted_crc = *(CRC *)&received_firmware[firmware_size - sizeof(CRC)];
    CRC received_crc    = calc_crc(received_firmware, firmware_size - sizeof(CRC));
    if (received_crc != transmitted_crc)
    {
        vLoggingPrintf("Firmware CRC mismatch: %u != %u\n", received_crc, transmitted_crc);
        vPortFree(received_firmware);
        return;
    }

    make_message(&ack_msg,      /* Destination message. */
                 m_seq++,       /* Message ID number. */
                 "sync_ack",    /* Message name. */
                 nullptr,       /* Message data, set to zero. */
                 Message_t::ACK /* Command. */
    );

    // Send acknowledgement
    if (send_message(&ack_msg) < 0)
    {
        vPortFree(received_firmware);
        close();
        return;
    }

    puts("Received OK!");

    std::string filename = System::Impl::get_executable_path();

    struct stat file_status = {};
    stat(filename.c_str(), &file_status);
    unlink(filename.c_str());

    std::fstream output_f(filename, std::ios::out | std::ios::trunc | std::ios::binary);
    output_f.write((char *)&received_firmware[0], success_count);

    vPortFree(received_firmware);

    if (!output_f.good())
    {
        vLoggingPrintf("Error writing to file!");
        return;
    }

    output_f.close();

    // Retain the file permissions
    chown(filename.c_str(), file_status.st_uid, file_status.st_gid);
    chmod(filename.c_str(), file_status.st_mode);
}

} // namespace Impl
} // namespace System
