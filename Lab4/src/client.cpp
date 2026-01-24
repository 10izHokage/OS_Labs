// Симулятор температурного устройства.
// Отправляет значения температуры по UDP на указанный ip:port.

#include <stdlib.h>     // atoi
#include <stdio.h>      
#include <string.h>     // memset
#include <time.h>       // time
#include <iostream>     // std::cout
#include <chrono>
#include <thread>

#if defined (WIN32)
#   include <winsock2.h>
#   pragma comment(lib, "ws2_32.lib")
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   define SOCKET int
#   define INVALID_SOCKET -1
#   define SOCKET_ERROR -1
#endif

static SOCKET g_sock = INVALID_SOCKET;

static void close_socket()
{
    if (g_sock == INVALID_SOCKET)
        return;
#if defined(WIN32) || defined(_WIN32)
    shutdown(g_sock, SD_BOTH);
    closesocket(g_sock);
    WSACleanup();
#else
    shutdown(g_sock, SHUT_RDWR);
    close(g_sock);
#endif
    g_sock = INVALID_SOCKET;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: client ip port\n";
        return -1;
    }

    const char* ip_str = argv[1];
    int port = atoi(argv[2]);
	
#if defined(WIN32) || defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock == INVALID_SOCKET) {
        std::cout << "Cant open socket!\n";
        return -1;
    }

    sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(ip_str);
    remote_addr.sin_port = htons((unsigned short)port);

    const int SEND_PERIOD_MS = 1000; 
    const double T_MIN = 20.0;
    const double T_MAX = 30.0;

    char out_buf[64];

    srand((unsigned)time(NULL));

    std::cout << "Temp sensor simulator started.\n";
    std::cout << "Sending UDP to " << ip_str << ":" << port << "\n";
    std::cout << "One value per second. Press Ctrl+C to stop.\n";

    for (;;) {
        double r = (double)rand() / (double)RAND_MAX;
        double temp = T_MIN + (T_MAX - T_MIN) * r;
        sprintf(out_buf, "%.2f", temp);

        int sended = sendto(g_sock, out_buf, (int)strlen(out_buf) + 1, 0,
                            (sockaddr*)&remote_addr, (int)sizeof(remote_addr));

        if (sended == 0 || sended == SOCKET_ERROR) {
            std::cout << "Error while send!\n";
        } else {
            std::cout << "Sent temperature: " << out_buf << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SEND_PERIOD_MS));
    }

    close_socket();
    return 0;
}
