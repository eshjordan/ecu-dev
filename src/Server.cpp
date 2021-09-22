#include "Server.hpp"

bool System::Impl::Server::server_started    = false;
Socket_t System::Impl::Server::server_socket = nullptr;

void System::Impl::Server::init(void)
{
    static bool inited = false;

    /* Ensure this is only run once, causes runtime error otherwise. */
    if (inited) { return; }

    /* Initialise the RTOS's TCP/IP stack. The tasks that use the network are created in the
     * vApplicationIPNetworkEventHook() hook function below. The hook function is called when the network connects.
     */
    FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
    inited = true;
}

void System::Impl::Server::start_task(void *arg)
{
    /* Create a new socket. */
    server_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

    if (!server_socket) { throw std::runtime_error("Socket failed to create!"); }

    /* Set the send and receive timouts for the socket. */
    static constexpr TickType_t xTimeOut = pdMS_TO_TICKS(2000);
    FreeRTOS_setsockopt(server_socket, 0, FREERTOS_SO_RCVTIMEO, (void *)&xTimeOut, 0);
    FreeRTOS_setsockopt(server_socket, 0, FREERTOS_SO_SNDTIMEO, (void *)&xTimeOut, 0);

    /* Use a sockaddr struct to set the server address. */
    freertos_sockaddr server_address{};
    server_address.sin_port = FreeRTOS_htons(9999U);

    /* Bind the socket to the port. */
    BaseType_t fail = FreeRTOS_bind(server_socket, &server_address, sizeof(freertos_sockaddr));

    if (fail)
    {
        FreeRTOS_closesocket(server_socket);
        throw std::runtime_error("Socket failed to bind to address!");
    }

    /* Log the network statistics to console. */
    print_network_stats();

    server_started = true;

    vLoggingPrintf("Server started!\n");

    vTaskDelete(NULL);
}

void System::Impl::Server::start(void)
{
    vLoggingPrintf("Network Up!\n");

    /* Create a task to set up the server socket when the RTOS Scheduler starts running. */
    xTaskCreate(start_task, "bindHook", 10000, /* Stack size in words, not bytes. */
                nullptr,                       /* Parameter passed into the task. */
                tskIDLE_PRIORITY,              /* Priority of the task. */
                nullptr);                      /* Don't need to keep the task handle. */
}
