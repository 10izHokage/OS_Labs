#include "serial_port.h"

#include <string.h>
#include <iostream>

#if !defined(WIN32) && !defined(_WIN32)
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   define SOCKET int
#   define INVALID_SOCKET -1
#   define SOCKET_ERROR -1
#endif

static SOCKET g_sock = INVALID_SOCKET;

bool serial_port_open(const char* ip, int port)
{
    if (g_sock != INVALID_SOCKET)
        return true;

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock == INVALID_SOCKET) {
        std::cout << "Cant open socket!\n";
        return false;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons((unsigned short)port);

    if (inet_pton(AF_INET, ip, &local_addr.sin_addr) <= 0) {
        std::cout << "Invalid IP address!\n";
        serial_port_close();
        return false;
    }

    if (bind(g_sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cout << "Failed to bind!\n";
        serial_port_close();
        return false;
    }

    return true;
}

bool serial_port_read_line(char* out_buf, int out_buf_size)
{
    if (!out_buf || out_buf_size <= 1)
        return false;
    if (g_sock == INVALID_SOCKET)
        return false;

    sockaddr_in remote_addr{};
    socklen_t addrlen = sizeof(remote_addr);

    int readd = recvfrom(g_sock, out_buf, out_buf_size - 1, 0,
                         (sockaddr*)&remote_addr, &addrlen);

    if (readd == SOCKET_ERROR || readd <= 0)
        return false;

    out_buf[readd] = '\0';
    return true;
}

void serial_port_close()
{
    if (g_sock != INVALID_SOCKET) {
        close(g_sock);
        g_sock = INVALID_SOCKET;
    }
}
