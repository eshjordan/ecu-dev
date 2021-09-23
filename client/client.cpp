#include <arpa/inet.h> //inet_addr
#include <csignal>
#include <cstdlib>
#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <unistd.h>

int socket_desc;

void error(char *message)
{
    shutdown(socket_desc, SHUT_RDWR);
    close(socket_desc);
    perror(message);
    exit(EXIT_FAILURE);
}

void sigpipe_handler(int unused) { printf("SIGPIPE caught\n"); }

int main(int argc, char *argv[])
{
    signal(SIGPIPE, sigpipe_handler);

    struct sockaddr_in server;
    char *message, server_reply[2000];

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) { error("Could not create socket"); }

    server.sin_addr.s_addr = inet_addr("192.168.10.33");
    server.sin_family      = AF_INET;
    server.sin_port        = htons(9999);

    // Connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) { error("connect error"); }

    puts("Connected\n");

    // Send some data
    message = "GET / HTTP/1.1\n\n";
    if (send(socket_desc, message, strlen(message), 0) < 0) { error("Send failed"); }
    puts("Data Send\n");

    // Receive a reply from the server
    if (recv(socket_desc, server_reply, 2000, 0) < 0) { error("recv failed"); }
    puts("Reply received\n");
    puts(server_reply);

    shutdown(socket_desc, SHUT_RDWR);
    close(socket_desc);

    return 0;
}
