#include "CAN_Msg.h"
#include "ECU_Error.h"
#include "ESP32_Msg.h"
#include "Header.h"
#include "Interproc_Msg.h"
#include "Parameter.h"
#include "Timestamp.h"
#include "types.h"
#include <arpa/inet.h>
#include <cassert>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#define ipconfigTCP_MSS 1400

/* Connection handlers */
int init();

void disconnect();

int reconnect();

/* Interactions */
void send_file(const std::filesystem::directory_entry &file, const std::string &type);

void echo_test();

void sync_test();

void remote_restart();

void request_status();

void firmware_update_test();

void software_update_test();

/* Client control */
void restart();

void end(int signal);
