#include "serial_port.h"

#include <string.h>
#include <iostream>

#if defined(WIN32) || defined(_WIN32)
#   include <winsock2.h>
#   pragma comment(lib, "ws2_32.lib")
#else
#   error "serial_port_win.cpp должен собираться только на Windows"
#endif

static SOCKET g_sock = INVALID_SOCKET;
static bool g_inited = false;

bool serial_port_open(const char* ip, int port)
{
    if (g_sock != INVALID_SOCKET)
        return true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "WSAStartup failed!\n";
        return false;
    }
    g_inited = true;

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock == INVALID_SOCKET) {
        std::cout << "Cant open socket!\n";
        serial_port_close();
        return false;
    }

    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(ip);
    local_addr.sin_port = htons((unsigned short)port);

    if (bind(g_sock, (sockaddr*)&local_addr, sizeof(local_addr))) {
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

    sockaddr_in remote_addr;
    int addrlen = sizeof(remote_addr);
    memset(&remote_addr, 0, sizeof(remote_addr));

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
        shutdown(g_sock, SD_BOTH);
        closesocket(g_sock);
        g_sock = INVALID_SOCKET;
    }
    if (g_inited) {
        WSACleanup();
        g_inited = false;
    }
}
