#include "Server.hpp"
#include "RTOS.hpp"
#include "RTOS_IP.hpp"
#include "System.hpp"
#include "portmacro.h"
#include "projdefs.h"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>

#define SERVER_PORT_NUM 8000

bool System::Impl::Server::server_started = false;
Socket_t System::Impl::Server::client_socket = nullptr;
TaskHandle_t System::Impl::Server::listen_task_handle = nullptr;

void vStartTCPEchoClientTasks_SingleTasks(uint16_t usTaskStackSize,
                                          UBaseType_t uxTaskPriority);

void System::Impl::Server::init(void) {
  static bool inited = false;

  /* Ensure this is only run once, causes runtime error otherwise. */
  if (inited) {
    return;
  }

  /* Initialise the RTOS's TCP/IP stack. The tasks that use the network are
   * created in the vApplicationIPNetworkEventHook() hook function below. The
   * hook function is called when the network connects.
   */
  FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress,
                  ucMACAddress);
  inited = true;
}

void System::Impl::Server::start_task(void *arg) {
  /* Log the network statistics to console. */
  print_network_stats();

  /* Create a new socket. */
  client_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                                  FREERTOS_IPPROTO_TCP);
  while (client_socket == FREERTOS_INVALID_SOCKET) {
    vLoggingPrintf("Failed to create socket, retrying...\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    client_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                                    FREERTOS_IPPROTO_TCP);
  }

  vLoggingPrintf("Socket created.\n");

  /* Set the send and receive timouts for the socket. */
  static constexpr TickType_t xTimeOut = pdMS_TO_TICKS(4000);
  FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_RCVTIMEO, (void *)&xTimeOut,
                      sizeof(xTimeOut));
  FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_SNDTIMEO, (void *)&xTimeOut,
                      sizeof(xTimeOut));

  /* Fill in the buffer and window sizes that will be used by the socket. */
  //   WinProperties_t winProps;
  //   winProps.lTxBufSize = 6 * ipconfigTCP_MSS;
  //   winProps.lTxWinSize = 3;
  //   winProps.lRxBufSize = 6 * ipconfigTCP_MSS;
  //   winProps.lRxWinSize = 3;
  //   /* Set the window and buffer sizes. */
  //   FreeRTOS_setsockopt(client_socket, 0, FREERTOS_SO_WIN_PROPERTIES,
  //                       (void *)&winProps, sizeof(winProps));

  /* Use a sockaddr struct to set the server address. */
  freertos_sockaddr server_address{};
  server_address.sin_port = FreeRTOS_htons(SERVER_PORT_NUM);
  server_address.sin_addr = FreeRTOS_GetIPAddress();

  /* Bind the socket to a port. */
  long bindStatus = 0;
  bindStatus =
      FreeRTOS_bind(client_socket, &server_address, sizeof(freertos_sockaddr));

  while (bindStatus != 0) {
    vLoggingPrintf("Failed to bind socket to address: ");
    switch (bindStatus) {
    case -FREERTOS_EINVAL: {
      vLoggingPrintf("The socket did not get bound, probably because the "
                     "specified port number was already in use.\n");
      break;
    }
    default: {
      vLoggingPrintf("The calling RTOS task did not get a response from the IP "
                     "RTOS task to the bind request.\n");
      break;
    }
    }
    vLoggingPrintf("Retrying...\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    bindStatus =
        FreeRTOS_bind(client_socket, nullptr, sizeof(freertos_sockaddr));
  }

  vLoggingPrintf("Bound socket successfully.\n");

  /* Connect to the server. */
  BaseType_t connectVal = 0;
  //   connectVal =
  //       FreeRTOS_connect(client_socket, &server_address,
  //       sizeof(server_address));

  while (connectVal != 0) {

    vLoggingPrintf("Failed to connect to server: ");

    switch (connectVal) {
    case -pdFREERTOS_ERRNO_EBADF: {
      vLoggingPrintf("Not a valid TCP socket.\n");
      break;
    }
    case -pdFREERTOS_ERRNO_EISCONN: {
      vLoggingPrintf("Socket was already connected before FreeRTOS_connect() "
                     "was called.\n");
      break;
    }
    case -pdFREERTOS_ERRNO_EINPROGRESS: {
      vLoggingPrintf(
          "Socket not in a state that allows a connect operation.\n");
      break;
    }
    case -pdFREERTOS_ERRNO_EWOULDBLOCK: {
      vLoggingPrintf("Socket has a read block time of zero and the connect "
                     "operation cannot succeed immediately.\n");
      break;
    }
    case -pdFREERTOS_ERRNO_ETIMEDOUT: {
      vLoggingPrintf("Connect attempt timed out.\n");
      break;
    }
    }

    vLoggingPrintf("Retrying...\n");

    vTaskDelay(pdMS_TO_TICKS(100));

    connectVal = FreeRTOS_connect(client_socket, &server_address,
                                  sizeof(server_address));
  }

  vLoggingPrintf("Socket is connected!\n");

  server_started = true;

  vTaskDelete(NULL);
}

void System::Impl::Server::listen_task(void *arg) {
  freertos_sockaddr client_address{};
  uint32_t address_size = sizeof(freertos_sockaddr);
  Socket_t connected_socket = 0;

  long listenStatus = 0;

  listenStatus = FreeRTOS_listen(client_socket, 20);

  while (listenStatus != 0) {
    switch (listenStatus) {
    case -pdFREERTOS_ERRNO_EOPNOTSUPP: {
      vLoggingPrintf("Socket is not a valid TCP socket or socket is not in "
                     "bound but closed state. Retrying...\n");
      break;
    }
    default: {
      vLoggingPrintf("Listen error. Retrying...\n");
      break;
    }
    }

    listenStatus = FreeRTOS_listen(client_socket, 20);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  while (true) {
    if (server_started) {
      vLoggingPrintf("Listening!\n");

      connected_socket =
          FreeRTOS_accept(client_socket, &client_address, &address_size);

      if (connected_socket == FREERTOS_INVALID_SOCKET) {
        vLoggingPrintf("Socket is not a valid TCP socket, or socket is not in "
                       "the Listening state. %hu\n",
                       connected_socket);
      } else if (connected_socket == nullptr) {
        vLoggingPrintf("Connection timeout.\n");
      } else {
        vLoggingPrintf("Client connected!\n");
        /* Spawn a RTOS task to handle the connection. */

        void **connection_info = (void **)pvPortMalloc(2 * sizeof(void *));
        connection_info[0] = (void *)connected_socket;
        connection_info[1] = (void *)&client_address;

        xTaskCreate(
            connection_task, "connection_task",
            10000,                   /* Stack size in words, not bytes. */
            (void *)connection_info, /* Parameter passed into the task. */
            tskIDLE_PRIORITY,        /* Priority of the task. */
            nullptr);                /* Task handle. */
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void System::Impl::Server::connection_task(void *arg) {
  auto container = (void **)arg;
  Socket_t connected_socket = (Socket_t)container[0];
  freertos_sockaddr client_address = *(freertos_sockaddr *)container[1];
  static char rx_data[ipconfigNETWORK_MTU];
  BaseType_t bytes_received = 0;

  vLoggingPrintf("Connection opened! - %s\n",
                 ip_to_str(client_address.sin_addr));

  while (true) {

    bytes_received =
        FreeRTOS_recv(connected_socket, &rx_data, ipconfigNETWORK_MTU, 0);

    FreeRTOS_send(connected_socket, (void *)&rx_data, bytes_received, 0);

    if (bytes_received > 0) {
      /* Data was received, process it here. */
      vLoggingPrintf("Tick!\n");
      vLoggingPrintf("My data: %s\n", rx_data);

      auto out = split(rx_data, '/');

      std::cout << "Parameters:\n";
      for (auto &i : out) {
        std::cout << i << std::endl;
      }
      std::cout << "\n";

      if (out.size() > 0) {
        if (out[0] == "ecu") {
          if (out.size() > 1) {
            if (out[1] == "param") {
              if (out.size() > 2) {
                auto name = out[2];
                if (out.size() > 3) {
                  if (out[3] == "get") {
                    vLoggingPrintf("Parameter %s: %lf\n", name.c_str(),
                                   System::get_parameter<double>(name));
                  }
                  if (out[3] == "set" && out.size() > 4) {
                    double value = std::stod(out[4]);
                    System::set_parameter(name, value);
                    vLoggingPrintf("Parameter %s set to %.2lf\n", name.c_str(),
                                   value);
                  }
                }
              }
            }
          }
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }

  vTaskDelete(nullptr);
}

void System::Impl::Server::start(void) {
  vLoggingPrintf("Network Up!\n");

  /* Create a task to set up the server socket when the RTOS Scheduler starts
   * running. */
  xTaskCreate(start_task, "start_task",
              10000,            /* Stack size in words, not bytes. */
              nullptr,          /* Parameter passed into the task. */
              tskIDLE_PRIORITY, /* Priority of the task. */
              nullptr);         /* Don't need to keep the task handle. */

  xTaskCreate(listen_task, "listen_task",
              10000,                /* Stack size in words, not bytes. */
              nullptr,              /* Parameter passed into the task. */
              tskIDLE_PRIORITY,     /* Priority of the task. */
              &listen_task_handle); /* Keep the task handle. */
}

void System::Impl::Server::shutdown(void) {
  vTaskDelete(listen_task_handle);
  FreeRTOS_shutdown(client_socket, FREERTOS_SHUT_RDWR);
  FreeRTOS_closesocket(client_socket);
  vLoggingPrintf("Network Down!\n");
}
