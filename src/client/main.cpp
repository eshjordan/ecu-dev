#include "Message.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

int main()
{
    char b[8];
    gets(b);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {

        puts("open error");
        return 1;
    }

    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr("10.0.0.199");
    server.sin_family      = AF_INET;
    server.sin_port        = htons(8000);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    Message_t msg;
    msg.id = 1;
    strncpy(msg.name, "Test Message", 64);
    msg.command       = Message_t::VALUE;
    *(long *)msg.data = 123456789;

    printf("Received Message:\nName: %s\nID: %u\nCommand: %u\nData: %ld\n",
        msg.name,
        msg.id,
        msg.command,
        *(long*)msg.data
    );

    timespec_get(&msg.stamp, TIME_UTC);
    calc_checksum(&msg);

    if (send(sock, &msg, sizeof(Message_t), 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("Sent ok!");

    printf("Current time: %ld.%09ld UTC\n", msg.stamp.tv_sec, msg.stamp.tv_nsec);

    return 0;
}
