#include "Connection.hpp"
#include "Message.h"
#include "System.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace System {
namespace Impl {

void Connection::send_status(Message_t *message)
{
    const char task_list_header[] = "Task Status\n"
                                    "Name         State   Priority  StackHighWaterMark  Num \n"
                                    "*******************************************************\n";

    const char runtime_stats_header[] = "\nTask Timings\n"
                                        "Name          Ticks         Percentage \n"
                                        "*************************************************\n";

    constexpr uint16_t task_print_size = 40 * 100;
    constexpr uint16_t msg_size =
        sizeof(task_list_header) + task_print_size + sizeof(runtime_stats_header) + task_print_size;

    Header_t header = {};
    make_header(&header, m_seq++, get_time_now(), msg_size);

    ssize_t tx = FreeRTOS_send(m_socket, &header, sizeof(header), 0);

    if (tx < 0)
    {
        printf("Header send failed\n");
        return;
    }

    char status_buffer[msg_size] = {0};
    uint offset                  = 0;

    // Copy task info header into buffer
    strncpy(status_buffer, task_list_header, sizeof(task_list_header));
    offset += sizeof(task_list_header);

    // Copy detailed task info into buffer
    vTaskList(status_buffer + offset);

    printf("%s\n\n\n", status_buffer);

    offset = msg_size / 2;

    // Copy task timing header into buffer
    strncpy(status_buffer + offset, runtime_stats_header, sizeof(runtime_stats_header));
    offset += sizeof(runtime_stats_header);

    // Copy task timing info into buffer
    vTaskGetRunTimeStats(status_buffer + offset);

    printf("%s\n\n\n", status_buffer + offset);

    auto bytes_to_send = msg_size;
    int tx_bytes       = 0;
    while (tx_bytes < msg_size)
    {
        ssize_t tx = FreeRTOS_send(m_socket, status_buffer + tx_bytes,
                                   bytes_to_send < ipconfigTCP_MSS ? bytes_to_send : ipconfigTCP_MSS, 0);

        if (tx < 0)
        {
            printf("send %d failed\n", tx_bytes);
            return;
        }

        tx_bytes += tx;
        bytes_to_send -= tx;
    }
}

void Connection::synchronize_connection(Message_t *message)
{
    auto num_messages = *(uint64_t *)&message->data;

    vLoggingPrintf("Synchronizing connection to %s, with %lld msgs...\n", m_ip_str, num_messages);

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

    vLoggingPrintf("Time offset - %s: %lld.%lld sec\n", m_ip_str, seconds, nanoseconds);

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
    auto num_files = *(uint64_t *)&message->data;

    vLoggingPrintf("Downloading %lld files...\n", num_files);

    Message_t ack_msg;
    make_message(&ack_msg,       /* Destination message. */
                 m_seq++,        /* Message ID number. */
                 "firmware_ack", /* Message name. */
                 nullptr,        /* Message data, set to zero. */
                 Message_t::ACK  /* Command. */
    );

    // Send acknowledgement
    if (send_message(&ack_msg) < 0)
    {
        vLoggingPrintf("Error sending ACK!");
        close();
        return;
    }

    for (int file_num = 0; file_num < num_files; file_num++)
    {
        Message_t file_header;
        receive_message(&file_header);

        std::string type_and_name = file_header.name;
        std::string file_type     = type_and_name.substr(0, type_and_name.find_first_of('/'));
        std::string filename      = type_and_name.substr(type_and_name.find_first_of('/') + 1);

        auto firmware_size = ((uint32_t *)&file_header.data)[0];
        auto firmware_crc  = ((uint32_t *)&file_header.data)[1];

        vLoggingPrintf("Downloading %s file %s...\n", file_type.c_str(), filename.c_str());
        vLoggingPrintf("Firmware size: %u\n", firmware_size);
        vLoggingPrintf("Firmware CRC: %u\n", firmware_crc);

        make_message(&ack_msg,      /* Destination message. */
                     m_seq++,       /* Message ID number. */
                     "sync_ack",    /* Message name. */
                     nullptr,       /* Message data, set to zero. */
                     Message_t::ACK /* Command. */
        );

        // Send acknowledgement
        if (send_message(&ack_msg) < 0)
        {
            vLoggingPrintf("Error sending ACK!");
            close();
            return;
        }

        uint8_t received_firmware[20 * 1024 * 1024]; // 20MB max firmware size

        if (firmware_size > sizeof(received_firmware))
        {
            vLoggingPrintf("File is too large!");
            close();
            return;
        }

        BaseType_t success_count = 0;
        int failed_count         = 0;
        int i                    = 0;
        while (success_count < firmware_size)
        {
            BaseType_t rx_bytes = FreeRTOS_recv(m_socket, received_firmware + success_count, ipconfigTCP_MSS, 0);

            if (rx_bytes < 0)
            {
                print_recv_err(rx_bytes);
                close();
                return;
            }

            success_count += rx_bytes;

            i++;
        }

        vLoggingPrintf("Received %ld bytes\n", success_count);

        // Check CRC
        CRC transmitted_crc = *(CRC *)&received_firmware[firmware_size - sizeof(CRC)];
        CRC received_crc    = calc_crc(received_firmware, firmware_size - sizeof(CRC));
        if (received_crc != transmitted_crc)
        {
            vLoggingPrintf("Firmware CRC mismatch: %u != %u\n", received_crc, transmitted_crc);
            return;
        }

        puts("Received OK!");

        std::string filepath;
        std::string base_dir = "/home/pi/ecu/";

        if (file_type == "bin")
        {
            filepath = base_dir + "bin/" + filename;
        } else if (file_type == "lib")
        {
            filepath = base_dir + "lib/" + filename;
        } else
        {
            vLoggingPrintf("Unknown file type: %s\n", file_type.c_str());
            return;
        }

        printf("Writing to %s\n", filepath.c_str());

        struct stat file_status = {};
        stat(filepath.c_str(), &file_status);
        unlink(filepath.c_str());

        FILE *output_f = fopen(filepath.c_str(), "wb");
        if (output_f == nullptr)
        {
            vLoggingPrintf("Error opening file %s for writing!\n", filepath.c_str());
            fclose(output_f);
            return;
        }

        if (fwrite(received_firmware, 1, success_count, output_f) != success_count)
        {
            puts("Error writing file!");
            fclose(output_f);
            return;
        }

        fclose(output_f);

        // Retain the file permissions
        chown(filepath.c_str(), file_status.st_uid, file_status.st_gid);
        chmod(filepath.c_str(), file_status.st_mode);

        make_message(&ack_msg,      /* Destination message. */
                     m_seq++,       /* Message ID number. */
                     "sync_ack",    /* Message name. */
                     nullptr,       /* Message data, set to zero. */
                     Message_t::ACK /* Command. */
        );

        // Send acknowledgement
        if (send_message(&ack_msg) < 0)
        {
            puts("Error sending ACK!");
            close();
            return;
        }
    }

    puts("Download complete!");
}

} // namespace Impl
} // namespace System
