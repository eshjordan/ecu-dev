#include "CRC.h"
#include "Message.h"
#include "RTOS_IP.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

int sock;
struct sockaddr_in server;
// struct sockaddr_in client;

#define ipconfigTCP_MSS 1400

int init();
void echo_test() {}
void sync_test();
void firmware_update_test();
void software_update_test() {}

int main()
{
    if (!init())
    {
        puts("Failed to initialize");
        return -1;
    }

    puts("Test options:\n"
         "\t1) echo_test\n"
         "\t2) sync_test\n"
         "\t3) firmware_update_test\n"
         "\t4) software_update_test\n"
         "\tq) quit\n");

    while (true)
    {
        std::string s;
        std::cin >> s;

        switch (s[0])
        {
        case '1': {
            echo_test();
            break;
        }
        case '2': {
            sync_test();
            break;
        }
        case '3': {
            firmware_update_test();
            break;
        }
        case '4': {
            software_update_test();
            break;
        }
        case 'q': {
            return 0;
        }
        default: {
            break;
        }
        }
    }

    return 0;
}

void sync_test()
{
    Message_t init_msg;
    uint32_t id         = 0;
    uint64_t sync_count = 10;

    make_message(&init_msg, id++, "Test Message", &sync_count, Message_t::SYNC);

    if (send(sock, &init_msg, sizeof(Message_t), 0) < 0)
    {
        puts("Init send failed");
        return;
    }

    uint8_t buf[1024];

    // Wait for server to acknowledge
    if (recv(sock, buf, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    // Start pinging
    for (int i = 0; i < sync_count; i++)
    {
        Message_t msg;
        make_message(&msg, id++, "Test Message", nullptr, Message_t::PING);

        if (send(sock, &msg, sizeof(Message_t), 0) < 0)
        {
            printf("send %d failed\n", i);
            return;
        }

        // Wait for server to acknowledge
        if (recv(sock, buf, sizeof(Message_t), 0) < 0)
        {
            puts("ACK recv failed");
            return;
        }
    }

    Message_t sync_status;
    ssize_t bytes_received = 0;

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Sec recv failed");
        return;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
        return;
    }

    auto seconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Nsec recv failed");
        return;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
        return;
    }

    auto nanoseconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if (nanoseconds < 0)
    {
        seconds--;
        nanoseconds += 1000000000;
    }

    printf("Round trip time diff: %ld.%09ld sec\n", seconds, nanoseconds);
}

void firmware_update_test()
{
    Message_t init_msg;
    uint32_t id = 0;

    std::string filename = "/home/jordan/Documents/2021/ecu-dev/bin/raspi/ecu_program";

    std::ifstream input_file(filename, std::ios::in | std::ios::binary);
    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    buf.reserve(buf.size() + sizeof(CRC));

    CRC firmware_crc = calc_crc(buf.data(), buf.size());

    for (int i = 0; i < sizeof(CRC); i++)
    {
        buf.push_back(((uint8_t *)&firmware_crc)[i]);
    }

    printf("Calced crc: %u\n", firmware_crc);

    uint8_t data[8];
    ((uint32_t *)data)[0] = buf.size();
    ((uint32_t *)data)[1] = firmware_crc;

    make_message(&init_msg, id++, "Test Message", data, Message_t::FIRMWARE_UPDATE);

    if (send(sock, &init_msg, sizeof(Message_t), 0) < 0)
    {
        puts("Init send failed");
        return;
    }

    Message_t ack_msg;

    // Wait for server to acknowledge
    if (recv(sock, &ack_msg, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    puts("recv OK!");

    // Start sending firmware
    auto bytes_to_send = buf.size();
    int tx_bytes       = 0;
    while (tx_bytes < buf.size())
    {

        ssize_t tx = send(sock, &buf[tx_bytes], bytes_to_send < ipconfigTCP_MSS ? bytes_to_send : ipconfigTCP_MSS, 0);

        if (tx < 0)
        {
            printf("send %d failed\n", tx_bytes);
            return;
        }

        tx_bytes += tx;
        bytes_to_send -= tx;
    }

    assert(tx_bytes == buf.size());

    // Wait for server to acknowledge
    if (recv(sock, &ack_msg, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    puts("Sent OK!");
}

int init()
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        puts("open error");
        return 0;
    }

    // client.sin_addr.s_addr = inet_addr("192.168.10.33"); // INADDR_LOOPBACK
    // client.sin_family      = AF_INET;
    // client.sin_port        = htons(8001);

    // if (bind(sock, (struct sockaddr *)&client, sizeof(client)) < 0)
    // {
    //     switch (errno)
    //     {
    //     case EACCES: {
    //         puts("EACCES");
    //         break;
    //     }
    //     case EADDRINUSE: {
    //         puts("EADDRINUSE");
    //         break;
    //     }
    //     case EBADF: {
    //         puts("EBADF");
    //         break;
    //     }

    //     case EINVAL: {
    //         puts("EINVAL");
    //         break;
    //     }
    //     case ENOTSOCK: {
    //         puts("ENOTSOCK");
    //         break;
    //     }
    //     case EADDRNOTAVAIL: {
    //         puts("EADDRNOTAVAIL");
    //         break;
    //     }
    //     case EFAULT: {
    //         puts("EFAULT");
    //         break;
    //     }
    //     case ELOOP: {
    //         puts("ELOOP");
    //         break;
    //     }
    //     case ENAMETOOLONG: {
    //         puts("ENAMETOOLONG");
    //         break;
    //     }
    //     case ENOENT: {
    //         puts("ENOENT");
    //         break;
    //     }
    //     case ENOMEM: {
    //         puts("ENOMEM");
    //         break;
    //     }

    //     case ENOTDIR: {
    //         puts("ENOTDIR");
    //         break;
    //     }

    //     case EROFS: {
    //         puts("EROFS");
    //         break;
    //     }
    //     default: {
    //         puts("default");
    //         break;
    //     }
    //     }

    //     puts("bind error");
    //     return 1;
    // }

    server.sin_addr.s_addr = inet_addr("192.168.10.116");
    server.sin_family      = AF_INET;
    server.sin_port        = htons(8000);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        switch (errno)
        {
        case EACCES: {
            puts("EACCES");
            break;
        }
        case EPERM: {
            puts("EPERM");
            break;
        }
        case EADDRINUSE: {
            puts("EADDRINUSE");
            break;
        }
        case EAGAIN: {
            puts("EAGAIN");
            break;
        }
        case ECONNREFUSED: {
            puts("ECONNREFUSED");
            break;
        }
        case EFAULT: {
            puts("EFAULT");
            break;
        }
        case EINPROGRESS: {
            puts("EINPROGRESS");
            break;
        }
        case EINTR: {
            puts("EINTR");
            break;
        }
        case EISCONN: {
            puts("EISCONN");
            break;
        }
        case ENETUNREACH: {
            puts("ENETUNREACH");
            break;
        }
        case EAFNOSUPPORT: {
            puts("EAFNOSUPPORT");
            break;
        }
        case EALREADY: {
            puts("EALREADY");
            break;
        }
        case EPROTOTYPE: {
            puts("EPROTOTYPE");
            break;
        }
        case ENOPROTOOPT: {
            puts("ENOPROTOOPT");
            break;
        }
        case ETIMEDOUT: {
            puts("ETIMEDOUT");
            break;
        }
        case EBADF: {
            puts("EBADF");
            break;
        }

        case EINVAL: {
            puts("EINVAL");
            break;
        }
        case ENOTSOCK: {
            puts("ENOTSOCK");
            break;
        }
        case EADDRNOTAVAIL: {
            puts("EADDRNOTAVAIL");
            break;
        }
        case ELOOP: {
            puts("ELOOP");
            break;
        }
        case ENAMETOOLONG: {
            puts("ENAMETOOLONG");
            break;
        }
        case ENOENT: {
            puts("ENOENT");
            break;
        }
        case ENOMEM: {
            puts("ENOMEM");
            break;
        }

        case ENOTDIR: {
            puts("ENOTDIR");
            break;
        }

        case EROFS: {
            puts("EROFS");
            break;
        }

        default: {
            puts("default");
            break;
        }
        }

        printf("connect error: %d\n", errno);
        return 0;
    }

    return 1;
}
