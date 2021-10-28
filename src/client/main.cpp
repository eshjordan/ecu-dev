#include "main.hpp"
#include "Message.h"
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

    if (!init()) { puts("Failed to initialize"); }

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
        puts("open error");
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
            puts("EACCES");
            break;
        }
        case EPERM: {
            puts("EPERM");
            break;
        }
        case EADDRINUSE: {
            puts("EADDRINUSE");
            break;
        }
        case EAGAIN: {
            puts("EAGAIN");
            break;
        }
        case ECONNREFUSED: {
            puts("ECONNREFUSED");
            break;
        }
        case EFAULT: {
            puts("EFAULT");
            break;
        }
        case EINPROGRESS: {
            puts("EINPROGRESS");
            break;
        }
        case EINTR: {
            puts("EINTR");
            break;
        }
        case EISCONN: {
            puts("EISCONN");
            break;
        }
        case ENETUNREACH: {
            puts("ENETUNREACH");
            break;
        }
        case EAFNOSUPPORT: {
            puts("EAFNOSUPPORT");
            break;
        }
        case EALREADY: {
            puts("EALREADY");
            break;
        }
        case EPROTOTYPE: {
            puts("EPROTOTYPE");
            break;
        }
        case ENOPROTOOPT: {
            puts("ENOPROTOOPT");
            break;
        }
        case ETIMEDOUT: {
            puts("ETIMEDOUT");
            break;
        }
        case EBADF: {
            puts("EBADF");
            break;
        }

        case EINVAL: {
            puts("EINVAL");
            break;
        }
        case ENOTSOCK: {
            puts("ENOTSOCK");
            break;
        }
        case EADDRNOTAVAIL: {
            puts("EADDRNOTAVAIL");
            break;
        }
        case ELOOP: {
            puts("ELOOP");
            break;
        }
        case ENAMETOOLONG: {
            puts("ENAMETOOLONG");
            break;
        }
        case ENOENT: {
            puts("ENOENT");
            break;
        }
        case ENOMEM: {
            puts("ENOMEM");
            break;
        }

        case ENOTDIR: {
            puts("ENOTDIR");
            break;
        }

        case EROFS: {
            puts("EROFS");
            break;
        }

        default: {
            puts("default");
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
        puts("Reconnected");
        return 1;
    }

    puts("Failed to reconnect");
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
        puts("Failed to open file");
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
        puts("fread error");
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

    Message_t file_header = {};
    make_message(&file_header, 0, type_and_name.c_str(), data, Message_t::VALUE);

    if (send(sock, &file_header, sizeof(Message_t), 0) < 0)
    {
        puts("File header send failed");
        return;
    }

    Message_t ack_msg;

    // Wait for server to acknowledge
    if (recv(sock, &ack_msg, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    puts("recv OK!");

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
    if (recv(sock, &ack_msg, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    puts("Sent OK!");
}

void echo_test()
{
    Message_t msg;
    make_message(&msg, 0, "ping", nullptr, Message_t::ECHO);
    send(sock, &msg, sizeof(Message_t), 0);
}

void sync_test()
{
    Message_t init_msg;
    uint32_t id         = 0;
    uint64_t sync_count = 10;

    make_message(&init_msg, id++, "Test Message", &sync_count, Message_t::SYNC);

    if (send(sock, &init_msg, sizeof(Message_t), 0) < 0)
    {
        puts("Init send failed");
        return;
    }

    uint8_t buf[1024];

    // Wait for server to acknowledge
    if (recv(sock, buf, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
        return;
    }

    // Start pinging
    for (int i = 0; i < sync_count; i++)
    {
        Message_t msg;
        make_message(&msg, id++, "Test Message", nullptr, Message_t::PING);

        if (send(sock, &msg, sizeof(Message_t), 0) < 0)
        {
            printf("send %d failed\n", i);
            return;
        }

        // Wait for server to acknowledge
        if (recv(sock, buf, sizeof(Message_t), 0) < 0)
        {
            puts("ACK recv failed");
            return;
        }
    }

    Message_t sync_status;
    ssize_t bytes_received = 0;

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Sec recv failed");
        return;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
        return;
    }

    auto seconds = *reinterpret_cast<int64_t *>(sync_status.data);

    if ((bytes_received = recv(sock, &sync_status, sizeof(Message_t), 0)) < 0)
    {
        puts("Nsec recv failed");
        return;
    }

    if (check_message(&sync_status) < 0)
    {
        puts("Received message is not a message");
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
    Message_t msg;
    make_message(&msg, 0, "restart", nullptr, Message_t::RESTART);
    send(sock, &msg, sizeof(Message_t), 0);
}

void request_status()
{
    Message_t msg;
    make_message(&msg, 0, "status", nullptr, Message_t::STATUS);
    send(sock, &msg, sizeof(Message_t), 0);

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

    Message_t init_msg;
    uint32_t id   = 0;
    uint64_t data = bin_files.size() + lib_files.size();
    make_message(&init_msg, id++, "Number of files", &data, Message_t::FIRMWARE_UPDATE);

    if (send(sock, &init_msg, sizeof(Message_t), 0) < 0)
    {
        puts("Command send failed");
        return;
    }

    Message_t ack_msg;
    if (recv(sock, &ack_msg, sizeof(Message_t), 0) < 0)
    {
        puts("ACK recv failed");
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

    puts("Restarting client...");
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

    puts("Restarting!");

    execl(client_filename, client_filename, (char *)nullptr);

    puts("Shutdown ok");

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