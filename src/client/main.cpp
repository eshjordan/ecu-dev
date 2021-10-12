#include "Message.h"
#include "RTOS_IP.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {

        puts("open error");
        return 1;
    }

    // struct sockaddr_in client;
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

    struct sockaddr_in server;
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
        return 1;
    }

    char b[8];
    gets(b);

    Message_t init_msg;
    uint32_t id         = 0;
    uint64_t sync_count = 10;

    make_message(&init_msg, id++, "Test Message", &sync_count, Message_t::SYNC);

    if (send(sock, &init_msg, sizeof(Message_t), 0) < 0)
    {
        puts("Init send failed");
        return 1;
    }

    uint8_t buf[1024];

    // Wait for server to acknowledge
    if (recv(sock, buf, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return 1;
    }

    // Start pinging
    for (int i = 0; i < sync_count; i++)
    {
        Message_t msg;
        make_message(&msg, id++, "Test Message", nullptr, Message_t::PING);

        if (send(sock, &msg, sizeof(Message_t), 0) < 0)
        {
            printf("send %d failed\n", i);
            return 1;
        }

        // Wait for server to acknowledge
        if (recv(sock, buf, sizeof(Message_t), 0) < 0)
        {
            puts("ACK recv failed");
            return 1;
        }
    }

    Message_t sync_status;
    ssize_t bytes_received = 0;

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Sec recv failed");
        return 1;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
        return 1;
    }

    auto seconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Nsec recv failed");
        return 1;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
        return 1;
    }

    auto nanoseconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if (nanoseconds < 0)
    {
        seconds--;
        nanoseconds += 1000000000;
    }

    printf("Round trip time diff: %ld.%09ld sec\n", seconds, nanoseconds);

    return 0;
}
