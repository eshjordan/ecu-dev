#include "Message.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

int main()
{
    char b[8];
    gets(b);

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

    Message_t msg;
    msg.header.id = 1;
    strncpy(msg.name, "Test Message", 64);
    msg.command       = Message_t::VALUE;
    *(long *)msg.data = 123456789;

    printf("Received Message:\nName: %s\nID: %u\nCommand: %u\nData: %ld\n", msg.name, msg.header.id, msg.command,
           *(long *)msg.data);

    timespec_get(&msg.header.stamp, TIME_UTC);
    calc_checksum(&msg);

    if (send(sock, &msg, sizeof(Message_t), 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("Sent ok!");

    printf("Current time: %ld.%09ld UTC\n", msg.header.stamp.tv_sec, msg.header.stamp.tv_nsec);

    return 0;
}
