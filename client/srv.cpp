#include <arpa/inet.h> //inet_addr
#include <csignal>
#include <cstdlib>
#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <unistd.h>

int socket_desc;
int client_sock;

void error(char *message)
{
    perror(message);
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    shutdown(socket_desc, SHUT_RDWR);
    close(socket_desc);
    exit(EXIT_FAILURE);
}

void sigpipe_handler(int unused) { printf("SIGPIPE caught\n"); }

void sigint_handler(int unused)
{
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    shutdown(socket_desc, SHUT_RDWR);
    close(socket_desc);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, sigpipe_handler);
    signal(SIGINT, sigint_handler);

    struct sockaddr_in server;
    char *message, server_reply[2000];

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) { printf("Could not create socket"); }

    server.sin_addr.s_addr = inet_addr("192.168.10.33");
    server.sin_family      = AF_INET;
    server.sin_port        = htons(9999);

    printf("IP: %u", server.sin_addr.s_addr);
    printf("Port: %u", server.sin_port);

    // Bind socket
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) { error("bind error"); }

    if (listen(socket_desc, 3) < 0) { error("listen error"); }

    puts("Connected\n");

    while (true)
    {
        /* Accept and incoming connection */
        puts("Waiting for incoming connections...");
        int c       = sizeof(struct sockaddr_in);
        client_sock = accept(socket_desc, (struct sockaddr *)&server, (socklen_t *)&c);
        if (client_sock < 0) { error("accept error"); }
        puts("Connection accepted");

        // Receive a message from the client
        if (recv(client_sock, server_reply, 2000, 0) <= 0) { error("recv failed"); }
        puts("Message received\n");
        puts(server_reply);

        // Send some data
        message = "GET / HTTP/1.1\n\n";
        if (send(client_sock, message, strlen(message), 0) < 0) { error("Send failed"); }
        puts("Data Send\n");

        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }

    return 0;
}
