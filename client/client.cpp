#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sockfd = 0;
    struct sockaddr_in servaddr{};

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr("192.168.10.33");

    int fail = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char sendline[1024];
    char recvline[1024];

    while (fgets(sendline, 1024, stdin) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        read(sockfd, recvline, 1024);
        printf("%s", recvline);
    }

    close(sockfd);

    return 0;
}
