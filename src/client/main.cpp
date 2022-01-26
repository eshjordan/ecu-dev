#include "main.hpp"
#include "ECU_Msg.h"
#include <algorithm>
#include <filesystem>

static int sock;
static struct sockaddr_in server;

static int s_argc = 0;
static std::vector<std::string> s_argv;

int main(int argc, char **argv)
{
    signal(SIGINT, end);

    s_argc = argc;
    for (int i = 0; i < argc; i++)
    {
        s_argv.emplace_back(argv[i]);
    }

    if (!init()) { printf("Failed to initialize\n"); }

    auto print_options = []() {
        puts("Test options:\n"
             "\t1) make and restart\n"
             "\t2) reconnect\n"
             "\t3) remote_restart\n"
             "\t4) request_status\n"
             "\t5) sync_test\n"
             "\t6) firmware_update_test\n"
             "\t7) software_update_test\n"
             "\tq) quit\n");
    };

    print_options();

    while (true)
    {
        std::string s;
        std::cin >> s;

        switch (s[0])
        {
        case '1': {
            disconnect();
            restart();
            print_options();
            break;
        }
        case '2': {
            reconnect();
            print_options();
            break;
        }
        case '3': {
            remote_restart();
            print_options();
            break;
        }
        case '4': {
            request_status();
            print_options();
            break;
        }
        case '5': {
            sync_test();
            print_options();
            break;
        }
        case '6': {
            firmware_update_test();
            print_options();
            break;
        }
        case '7': {
            software_update_test();
            print_options();
            break;
        }
        case 'q': {
            end(0);
        }
        default: {
            break;
        }
        }
    }

    return 0;
}

/* Connection handlers */
int init()
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        printf("open error\n");
        return 0;
    }

    server.sin_addr.s_addr = inet_addr("192.168.10.116");
    server.sin_family      = AF_INET;
    server.sin_port        = htons(8000);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        switch (errno)
        {
        case EACCES: {
            printf("EACCES\n");
            break;
        }
        case EPERM: {
            printf("EPERM\n");
            break;
        }
        case EADDRINUSE: {
            printf("EADDRINUSE\n");
            break;
        }
        case EAGAIN: {
            printf("EAGAIN\n");
            break;
        }
        case ECONNREFUSED: {
            printf("ECONNREFUSED\n");
            break;
        }
        case EFAULT: {
            printf("EFAULT\n");
            break;
        }
        case EINPROGRESS: {
            printf("EINPROGRESS\n");
            break;
        }
        case EINTR: {
            printf("EINTR\n");
            break;
        }
        case EISCONN: {
            printf("EISCONN\n");
            break;
        }
        case ENETUNREACH: {
            printf("ENETUNREACH\n");
            break;
        }
        case EAFNOSUPPORT: {
            printf("EAFNOSUPPORT\n");
            break;
        }
        case EALREADY: {
            printf("EALREADY\n");
            break;
        }
        case EPROTOTYPE: {
            printf("EPROTOTYPE\n");
            break;
        }
        case ENOPROTOOPT: {
            printf("ENOPROTOOPT\n");
            break;
        }
        case ETIMEDOUT: {
            printf("ETIMEDOUT\n");
            break;
        }
        case EBADF: {
            printf("EBADF\n");
            break;
        }

        case EINVAL: {
            printf("EINVAL\n");
            break;
        }
        case ENOTSOCK: {
            printf("ENOTSOCK\n");
            break;
        }
        case EADDRNOTAVAIL: {
            printf("EADDRNOTAVAIL\n");
            break;
        }
        case ELOOP: {
            printf("ELOOP\n");
            break;
        }
        case ENAMETOOLONG: {
            printf("ENAMETOOLONG\n");
            break;
        }
        case ENOENT: {
            printf("ENOENT\n");
            break;
        }
        case ENOMEM: {
            printf("ENOMEM\n");
            break;
        }

        case ENOTDIR: {
            printf("ENOTDIR\n");
            break;
        }

        case EROFS: {
            printf("EROFS\n");
            break;
        }

        default: {
            printf("default\n");
            break;
        }
        }

        printf("connect error: %d\n", errno);
        return 0;
    }

    return 1;
}

void disconnect()
{
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

int reconnect()
{
    disconnect();
    if (init())
    {
        printf("Reconnected\n");
        return 1;
    }

    printf("Failed to reconnect\n");
    return 0;
}

/* Interactions */
void send_file(const std::filesystem::directory_entry &file, const std::string &type)
{
    auto filename = file.path().filename();

    std::cout << "Sending " << filename << "..." << std::endl;

    FILE *input_file = fopen(file.path().c_str(), "rb");
    if (!input_file)
    {
        printf("Failed to open file\n");
        return;
    }

    // get its size:
    auto file_size = file.file_size();
    std::cout << "The size of " << filename << " is " << file_size << " bytes.\n";

    // reserve capacity
    uint8_t *buf = (uint8_t *)malloc(file_size + sizeof(CRC));
    if (fread(buf, sizeof(uint8_t), file_size, input_file) != file_size)
    {
        fclose(input_file);
        printf("fread error\n");
        return;
    }

    fclose(input_file);

    std::cout << "File size: " << file_size << std::endl;

    CRC firmware_crc = calc_crc(buf, file_size);

    (CRC &)buf[file_size] = firmware_crc;
    file_size += sizeof(CRC);

    printf("Calced crc: %u\n", firmware_crc);

    uint8_t data[8];
    ((uint32_t *)data)[0] = file_size;
    ((uint32_t *)data)[1] = firmware_crc;

    std::string type_and_name = type + "/" + filename.c_str();

    ECU_Msg_t file_header = ecu_msg_make(0, type_and_name.c_str(), data, ECU_Msg_t::VALUE_CMD);

    if (send(sock, &file_header, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("File header send failed\n");
        return;
    }

    ECU_Msg_t ack_msg;

    // Wait for server to acknowledge
    if (recv(sock, &ack_msg, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("ACK recv failed\n");
        return;
    }

    printf("recv OK!\n");

    // Start sending firmware
    auto bytes_to_send = file_size;
    int tx_bytes       = 0;
    while (tx_bytes < file_size)
    {
        ssize_t tx = send(sock, &buf[tx_bytes], bytes_to_send < ipconfigTCP_MSS ? bytes_to_send : ipconfigTCP_MSS, 0);

        if (tx < 0)
        {
            printf("send %d failed\n", tx_bytes);
            return;
        }

        tx_bytes += tx;
        bytes_to_send -= tx;
    }

    assert(tx_bytes == file_size);

    // Wait for server to acknowledge
    if (recv(sock, &ack_msg, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("ACK recv failed\n");
        return;
    }

    printf("Sent OK!\n");
}

void echo_test()
{
    ECU_Msg_t msg = ecu_msg_make(0, "ping", nullptr, ECU_Msg_t::ECHO_CMD);
    send(sock, &msg, sizeof(ECU_Msg_t), 0);
}

void sync_test()
{
    uint32_t id         = 0;
    uint64_t sync_count = 10;

    ECU_Msg_t init_msg = ecu_msg_make(id++, "Test Message", &sync_count, ECU_Msg_t::SYNC_CMD);

    if (send(sock, &init_msg, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("Init send failed\n");
        return;
    }

    uint8_t buf[1024];

    // Wait for server to acknowledge
    if (recv(sock, buf, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("ACK recv failed\n");
        return;
    }

    // Start pinging
    for (int i = 0; i < sync_count; i++)
    {
        ECU_Msg_t msg = ecu_msg_make(id++, "Test Message", nullptr, ECU_Msg_t::PING_CMD);

        if (send(sock, &msg, sizeof(ECU_Msg_t), 0) < 0)
        {
            printf("send %d failed\n", i);
            return;
        }

        // Wait for server to acknowledge
        if (recv(sock, buf, sizeof(ECU_Msg_t), 0) < 0)
        {
            printf("ACK recv failed\n");
            return;
        }
    }

    ECU_Msg_t sync_status;
    ssize_t bytes_received = 0;

    if ((bytes_received = recv(sock, &sync_status, sizeof(ECU_Msg_t), 0)) < 0)
    {
        printf("Sec recv failed\n");
        return;
    }

    if (ecu_msg_check(&sync_status) < 0)
    {
        printf("Received message is not a message\n");
        return;
    }

    auto seconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if ((bytes_received = recv(sock, &sync_status, sizeof(ECU_Msg_t), 0)) < 0)
    {
        printf("Nsec recv failed\n");
        return;
    }

    if (ecu_msg_check(&sync_status) < 0)
    {
        printf("Received message is not a message\n");
        return;
    }

    auto nanoseconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if (nanoseconds < 0)
    {
        seconds--;
        nanoseconds += 1000000000;
    }

    printf("Round trip time diff: %ld.%09ld sec\n", seconds, nanoseconds);
}

void remote_restart()
{
    ECU_Msg_t msg = ecu_msg_make(0, "restart", nullptr, ECU_Msg_t::RESTART_CMD);
    send(sock, &msg, sizeof(ECU_Msg_t), 0);
}

void request_status()
{
    ECU_Msg_t msg = ecu_msg_make(0, "status", nullptr, ECU_Msg_t::STATUS_CMD);
    send(sock, &msg, sizeof(ECU_Msg_t), 0);

    Header_t header;
    recv(sock, &header, sizeof(Header_t), 0);

    for (int i = 0; i < header.length; i++)
    {
        char c;
        recv(sock, &c, 1, 0);
        std::cout << c;
    }
}

void firmware_update_test()
{
    std::string bin_path = "/home/jordan/Documents/2021/ecu-dev/bin/raspi";
    std::string lib_path = "/home/jordan/Documents/2021/ecu-dev/lib/raspi";

    std::vector<std::filesystem::directory_entry> bin_files;
    std::vector<std::filesystem::directory_entry> lib_files;

    for (const auto &file : std::filesystem::directory_iterator(bin_path))
    {
        if (file.is_directory() || file.is_socket() || file.is_fifo()) { continue; }

        bin_files.push_back(file);
    }

    for (const auto &file : std::filesystem::directory_iterator(lib_path))
    {
        if (file.is_directory() || file.is_socket() || file.is_fifo()) { continue; }

        lib_files.push_back(file);
    }

    uint32_t id        = 0;
    uint64_t data      = bin_files.size() + lib_files.size();
    ECU_Msg_t init_msg = ecu_msg_make(id++, "Number of files", &data, ECU_Msg_t::FIRMWARE_UPDATE_CMD);

    if (send(sock, &init_msg, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("Command send failed\n");
        return;
    }

    ECU_Msg_t ack_msg;
    if (recv(sock, &ack_msg, sizeof(ECU_Msg_t), 0) < 0)
    {
        printf("ACK recv failed\n");
        return;
    }

    for (const auto &file : bin_files)
    {
        send_file(file, "bin");
    }

    for (const auto &file : lib_files)
    {
        send_file(file, "lib");
    }

    // Disconnect from server
    shutdown(sock, SHUT_RDWR);
    close(sock);

    printf("Restarting client...\n");
}

void software_update_test() {}

/* Client control */
void restart()
{
    FILE *fp;
    system("");

    char path[1024];

    /* Open the command for reading. */
    fp = popen("/bin/bash /home/jordan/Documents/2021/ecu-dev/make", "r");

    if (fp == nullptr)
    {
        printf("Failed to run command\n");
        return;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != nullptr)
    {
        printf("%s", path);
    }

    /* close */
    pclose(fp);

    const char *client_filename = "/home/jordan/Documents/2021/ecu-dev/bin/amd64/ecu_client";

    struct stat file_status = {};
    stat(client_filename, &file_status);
    unlink(client_filename);

    // Retain the file permissions
    chown(client_filename, file_status.st_uid, file_status.st_gid);
    chmod(client_filename, file_status.st_mode);

    printf("Restarting!\n");

    execl(client_filename, client_filename, (char *)nullptr);

    printf("Shutdown ok\n");

    /* Restart the program */

    char **argv = new char *[s_argc + 1];
    for (int i = 0; i < s_argc; i++)
    {
        argv[i] = new char[s_argv[i].size() + 1];
        strcpy(argv[i], s_argv[i].c_str());
        std::cout << argv[i] << std::endl;
    }
    argv[s_argc] = nullptr;

    std::cout << "Restarting - " << argv[0] << std::endl;

    // Currently the child becomes an orphan. This is apparently ok? as it gets adopted by init at the highest level.
    bool parent = false;
    bool child  = false;

    pid_t pid_1 = fork();

    parent = pid_1 != 0;
    child  = pid_1 == 0;

    if (child)
    {
        setsid();
        execv(argv[0], argv);
    }

    if (child) { exit(0); }
}

void end(int signal)
{
    disconnect();
    exit(0);
}