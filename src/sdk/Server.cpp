#include "Server.hpp"
#include "Connection.hpp"
#include "Message.h"
#include "RTOS.hpp"
#include "RTOS_IP.hpp"
#include "System.hpp"
#include "portmacro.h"
#include "projdefs.h"
#include "utils.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>

#define SERVER_PORT_NUM 8000

using byte_t = uint8_t;

bool System::Impl::Server::server_started             = false;
Socket_t System::Impl::Server::server_socket          = nullptr;
TaskHandle_t System::Impl::Server::listen_task_handle = nullptr;
std::vector<std::shared_ptr<System::Impl::Connection>> System::Impl::Server::connections =
    std::vector<std::shared_ptr<System::Impl::Connection>>();

void System::Impl::Server::init(void)
{
    static bool inited = false;

    /* Ensure this is only run once, causes runtime error otherwise. */
    if (inited) { return; }

    /* Initialise the RTOS's TCP/IP stack. The tasks that use the network are
     * created in the vApplicationIPNetworkEventHook() hook function below. The
     * hook function is called when the network connects.
     */
    FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
    inited = true;
}

void System::Impl::Server::start_task(void *arg)
{
    /* Log the network statistics to console. */
    print_network_stats();

    /* Create a new socket. */
    server_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    while (server_socket == FREERTOS_INVALID_SOCKET)
    {
        vLoggingPrintf("Failed to create socket, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        server_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    }

    vLoggingPrintf("Socket created.\n");

    /* Set the send and receive timouts for the socket. */
    static constexpr TickType_t xTimeOut = pdMS_TO_TICKS(4000);
    FreeRTOS_setsockopt(server_socket, 0, FREERTOS_SO_RCVTIMEO, (void *)&xTimeOut, sizeof(xTimeOut));
    FreeRTOS_setsockopt(server_socket, 0, FREERTOS_SO_SNDTIMEO, (void *)&xTimeOut, sizeof(xTimeOut));

    /* Use a sockaddr struct to set the server address. */
    freertos_sockaddr server_address{};
    server_address.sin_port = FreeRTOS_htons(SERVER_PORT_NUM);
    server_address.sin_addr = FreeRTOS_GetIPAddress();

    /* Bind the socket to a port. */
    long bindStatus = 0;
    bindStatus      = FreeRTOS_bind(server_socket, &server_address, sizeof(freertos_sockaddr));

    while (bindStatus != 0)
    {
        print_bind_err(bindStatus);
        vLoggingPrintf("Retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        bindStatus = FreeRTOS_bind(server_socket, nullptr, sizeof(freertos_sockaddr));
    }

    vLoggingPrintf("Bound socket successfully.\n");

    server_started = true;

    vTaskDelete(NULL);
}

void System::Impl::Server::listen_task(void *arg)
{
    freertos_sockaddr client_address{};
    uint32_t address_size = sizeof(freertos_sockaddr);

    long listenStatus = FreeRTOS_listen(server_socket, 20);

    while (listenStatus != 0)
    {
        print_listen_err(listenStatus);
        listenStatus = FreeRTOS_listen(server_socket, 20);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    while (true)
    {
        if (server_started)
        {
            vLoggingPrintf("Listening!\n");

            Socket_t connected_socket = FreeRTOS_accept(server_socket, &client_address, &address_size);

            if (connected_socket && connected_socket != FREERTOS_INVALID_SOCKET)
            {
                // Create a new Connection instance
                std::shared_ptr<Connection> connection = std::make_shared<Connection>(connected_socket, client_address);
                connections.push_back(connection);
            } else
            {
                print_accept_err(connected_socket);
            }
        }

        // Clear dead connections
        for (auto connection = connections.begin(); connection != connections.end();)
        {
            if (!(*connection)->is_connected())
            {
                (*connection)->disconnect();
                connection = connections.erase(connection);
            } else
            {
                ++connection;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void System::Impl::Server::start(void)
{
    vLoggingPrintf("Network Up!\n");

    /* Create a task to set up the server socket when the RTOS Scheduler starts
     * running. */
    xTaskCreate(start_task, "start_task", 10000, /* Stack size in words, not bytes. */
                nullptr,                         /* Parameter passed into the task. */
                tskIDLE_PRIORITY,                /* Priority of the task. */
                nullptr);                        /* Don't need to keep the task handle. */

    xTaskCreate(listen_task, "listen_task", 10000, /* Stack size in words, not bytes. */
                nullptr,                           /* Parameter passed into the task. */
                tskIDLE_PRIORITY,                  /* Priority of the task. */
                &listen_task_handle);              /* Keep the task handle. */
}

void System::Impl::Server::shutdown(void)
{
    vTaskDelete(listen_task_handle);

    for (const auto &connection : connections)
    {
        connection->disconnect();
    }

    FreeRTOS_shutdown(server_socket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(server_socket);
    vLoggingPrintf("Network Down!\n");
}
