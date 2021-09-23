#include "Server.hpp"
#include "RTOS.hpp"
#include "portmacro.h"
#include "projdefs.h"
#include <sys/socket.h>

bool System::Impl::Server::server_started             = false;
Socket_t System::Impl::Server::client_socket          = nullptr;
TaskHandle_t System::Impl::Server::listen_task_handle = nullptr;

void vStartTCPEchoClientTasks_SingleTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority);

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
    /* Log the network statistics to console. */
//     print_network_stats();

//     while (true)
//     {

//         /* Create a new socket. */
//         client_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
//         if (client_socket == FREERTOS_INVALID_SOCKET) { throw std::runtime_error("Socket failed to create!"); }

//         /* Set the send and receive timouts for the socket. */
//         static constexpr TickType_t xTimeOut = pdMS_TO_TICKS(4000);
//         FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_RCVTIMEO, (void *)&xTimeOut, sizeof(xTimeOut));
//         FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_SNDTIMEO, (void *)&xTimeOut, sizeof(xTimeOut));

//         /* Fill in the buffer and window sizes that will be used by the socket. */
//         WinProperties_t winProps;
//         winProps.lTxBufSize = 6 * ipconfigTCP_MSS;
//         winProps.lTxWinSize = 3;
//         winProps.lRxBufSize = 6 * ipconfigTCP_MSS;
//         winProps.lRxWinSize = 3;
//         /* Set the window and buffer sizes. */
//         FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_WIN_PROPERTIES, (void *)&winProps, sizeof(winProps));

//         /* Bind the socket to the port. */
//         if (FreeRTOS_bind(client_socket, nullptr, sizeof(freertos_sockaddr)) != 0)
//         {
//             FreeRTOS_closesocket(client_socket);
//             throw std::runtime_error("Socket failed to bind to address!");
//         }

//         /* Use a sockaddr struct to set the server address. */
//         freertos_sockaddr server_address{};
//         #define PORT_NUM 8000
//         server_address.sin_port = FreeRTOS_htons(PORT_NUM);
//         server_address.sin_addr = FreeRTOS_inet_addr_quick(192, 168, 10, 112);

//         BaseType_t connectVal = 0;
//         connectVal            = FreeRTOS_connect(client_socket, &server_address, sizeof(server_address));

//         switch (connectVal)
//         {
//         case -pdFREERTOS_ERRNO_EBADF: {
//             vLoggingPrintf("not a valid TCP socket.\n");
//             break;
//         }
//         case -pdFREERTOS_ERRNO_EISCONN: {
//             vLoggingPrintf("was already connected before FreeRTOS_connect() was called.\n");
//             break;
//         }
//         case -pdFREERTOS_ERRNO_EINPROGRESS: {
//             vLoggingPrintf("not in a state that allows a connect operation: err%d.\n", connectVal);
//             break;
//         }
//         case -pdFREERTOS_ERRNO_EWOULDBLOCK: {
//             vLoggingPrintf(
//                 "socket has a read block time of zero and the connect operation cannot succeed immediately.\n");
//             break;
//         }
//         case -pdFREERTOS_ERRNO_ETIMEDOUT: {
//             vLoggingPrintf("connect attempt times out.\n");
//             break;
//         }
//         case 0: {
//             vLoggingPrintf("Socket is connected!\n");
//             break;
//         }
//         }

// #define BUFFER_SIZE 1400
//         char txString[BUFFER_SIZE] = {0};
//         char rxString[BUFFER_SIZE] = {0};
//         int xReturned              = 0;

//         if (connectVal == 0)
//         {
//             /* Send a number of echo requests. */
//             for (int loopCount = 0; loopCount < 1; loopCount++)
//             {
//                 /* Create the string that is sent to the echo server. */
//                 strncpy(txString, "Hello World!", sizeof("Hello World!"));
//                 int lStringLength = strlen(txString);

//                 printf("sending data to the echo server \n");
//                 /* Send the string to the socket. */
//                 int lTransmitted = FreeRTOS_send(client_socket,    /* The socket being sent to. */
//                                                  (void *)txString, /* The data being sent. */
//                                                  lStringLength,    /* The length of the data being sent. */
//                                                  0);               /* No flags. */

//                 if (lTransmitted < 0)
//                 {
//                     /* Error? */
//                     vLoggingPrintf("FreeRTOS_send() failed.\n");
//                     break;
//                 }

//                 /* Clear the buffer into which the echoed string will be
//                 placed. */
//                 memset((void *)rxString, 0x00, BUFFER_SIZE);
//                 int xReceivedBytes = 0;

//                 /* Receive data echoed back to the socket. */
//                 while (xReceivedBytes < lTransmitted)
//                 {
//                     xReturned = FreeRTOS_recv(
//                         client_socket,                  /* The socket being received from. */
//                         &(rxString[xReceivedBytes]),    /* The buffer into which the received data will be written.
//                                                          */
//                         lStringLength - xReceivedBytes, /* The size of the buffer provided to receive the data. */
//                         0);                             /* No flags. */

//                     if (xReturned < 0)
//                     {
//                         /* Error occurred.  Latch it so it can be detected
//                         below. */
//                         xReceivedBytes = xReturned;
//                         vLoggingPrintf("FreeRTOS_recv() failed.\n");
//                         break;
//                     } else if (xReturned == 0)
//                     {
//                         /* Timed out. */
//                         vLoggingPrintf("FreeRTOS_recv() timed out.\n");
//                         break;
//                     } else
//                     {
//                         /* Keep a count of the bytes received so far. */
//                         xReceivedBytes += xReturned;
//                     }
//                 }

//                 /* If an error occurred it will be latched in xReceivedBytes,
//                 otherwise xReceived bytes will be just that - the number of
//                 bytes received from the echo server. */
//                 if (xReceivedBytes > 0)
//                 {
//                     /* Compare the transmitted string to the received string. */
//                     configASSERT(strncmp(rxString, txString, lTransmitted) == 0);

//                     if (strncmp(rxString, txString, lTransmitted) == 0)
//                     {
//                         /* The echo reply was received without error. */
//                         vLoggingPrintf("Echo reply received without error.\n");
//                     } else
//                     {
//                         /* The received string did not match the transmitted
//                         string. */
//                         vLoggingPrintf("Echo reply did not match transmitted string.\n");
//                         break;
//                     }
//                 } else if (xReceivedBytes < 0)
//                 {
//                     /* FreeRTOS_recv() returned an error. */
//                     vLoggingPrintf("FreeRTOS_recv() failed.\n");
//                     break;
//                 } else
//                 {
//                     /* Timed out without receiving anything? */
//                     vLoggingPrintf("FreeRTOS_recv() timed out.\n");
//                     break;
//                 }
//             }

//             /* Finished using the connected socket, initiate a graceful close:
//             FIN, FIN+ACK, ACK. */
//             vLoggingPrintf("Closing socket.\n");
//             FreeRTOS_shutdown(client_socket, FREERTOS_SHUT_RDWR);

//             do
//             {
//                 xReturned = FreeRTOS_recv(client_socket,  /* The socket being received from. */
//                                           &(rxString[0]), /* The buffer into which the received data will be written. */
//                                           BUFFER_SIZE,    /* The size of the buffer provided to receive the data. */
//                                           0);

//                 if (xReturned < 0) { break; }
//             } while (true);
//         }

//         vLoggingPrintf("Looped!");
//         vTaskDelay(pdMS_TO_TICKS(150));
//     }

//     server_started = true;

//     vTaskDelete(NULL);
}

void System::Impl::Server::listen_task(void *arg)
{
    while (true)
    {
        if (server_started)
        {
            vLoggingPrintf("Listening!");
            freertos_sockaddr client_address{};
            uint32_t address_size     = sizeof(freertos_sockaddr);
            Socket_t connected_socket = FreeRTOS_accept(client_socket, &client_address, &address_size);

            if (connected_socket && connected_socket != FREERTOS_INVALID_SOCKET)
            {
                vLoggingPrintf("Client connected!\n");
                /* Spawn a RTOS task to handle the connection. */
                xTaskCreate(connection_task, "connection_task", 10000, /* Stack size in words, not bytes. */
                            (void *)connected_socket,                  /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,                          /* Priority of the task. */
                            nullptr);                                  /* Task handle. */
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void System::Impl::Server::connection_task(void *arg)
{
    while (true)
    {
        vLoggingPrintf("CONNECTED\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // Socket_t connected_socket = (Socket_t)arg;
    // static char rx_data[ipconfigNETWORK_MTU];
    // BaseType_t bytes_received = 0;

    // bytes_received = FreeRTOS_recv(connected_socket, &rx_data, ipconfigNETWORK_MTU, 0);
    // FreeRTOS_send(connected_socket, "Got Message!", 12, 0);

    // if (bytes_received > 0)
    // {
    //     /* Data was received, process it here. */
    //     vLoggingPrintf("Tick!\n");
    //     vLoggingPrintf("My data: %s\n", rx_data);
    // }

    // vTaskDelete(nullptr);
}

void System::Impl::Server::start(void)
{
    vLoggingPrintf("Network Up!\n");

    vStartTCPEchoClientTasks_SingleTasks(10000, tskIDLE_PRIORITY);

    // /* Create a task to set up the server socket when the RTOS Scheduler starts running. */
    // xTaskCreate(start_task, "start_task", 10000, /* Stack size in words, not bytes. */
    //             nullptr,                         /* Parameter passed into the task. */
    //             tskIDLE_PRIORITY,                /* Priority of the task. */
    //             nullptr);                        /* Don't need to keep the task handle. */

    // xTaskCreate(listen_task, "listen_task", 10000, /* Stack size in words, not bytes. */
    //             nullptr,                           /* Parameter passed into the task. */
    //             tskIDLE_PRIORITY,                  /* Priority of the task. */
    //             &listen_task_handle);              /* Keep the task handle. */
}

void System::Impl::Server::shutdown(void)
{
    // vTaskDelete(listen_task_handle);
    FreeRTOS_shutdown(client_socket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(client_socket);
    vLoggingPrintf("Network Down!\n");
}
