#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>

#include "serial_port.h"
#include "db.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

void close_socket(SOCKET s) {
#if defined(WIN32) || defined(_WIN32)
    closesocket(s);
#else
    close(s);
#endif
}

void http_server_thread(int port) {
    SOCKET srv = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(srv, (sockaddr*)&addr, sizeof(addr));
    listen(srv, 5);

    std::cout << "HTTP server started on port " << port << "\n";

    char buffer[2048];

    while (true) {
        SOCKET client = accept(srv, 0, 0);
        if (client == INVALID_SOCKET) continue;

        int r = recv(client, buffer, sizeof(buffer)-1, 0);
        if (r <= 0) { close_socket(client); continue; }
        buffer[r] = 0;

        std::string req(buffer);
        std::string line = req.substr(0, req.find("\r\n"));
        std::cout << "HTTP request: " << line << "\n";

        // Разбираем метод и путь
        std::string method, path;
        std::istringstream iss(line);
        iss >> method >> path;

        std::ostringstream resp;
        std::ostringstream body;

        // ==== Обработка запросов ==== 
        if (path.find("/current") == 0) {
            double temp = db_get_last_temperature();
            body << "{ \"temperature\": " << temp << " }";

            resp << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                 << "Content-Length: " << body.str().size() << "\r\n\r\n"
                 << body.str();
            send(client, resp.str().c_str(), resp.str().size(), 0);
        }
        else if (path.find("/hourly") == 0) {
            auto data = db_get_hourly_avg(); 

            body << "[";
            for (size_t i = 0; i < data.size(); ++i) {
                if (i) body << ",";
                body << "{ \"time\": \"" << data[i].first << "\", \"avg\": " << data[i].second << "}";
            }
            body << "]";

            resp << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                 << "Content-Length: " << body.str().size() << "\r\n\r\n"
                 << body.str();
            send(client, resp.str().c_str(), resp.str().size(), 0);
        }
        else if (path.find("/daily") == 0) {
            auto data = db_get_daily_avg(); 

            body << "[";
            for (size_t i = 0; i < data.size(); ++i) {
                if (i) body << ",";
                body << "{ \"date\": \"" << data[i].first << "\", \"avg\": " << data[i].second << "}";
            }
            body << "]";

            resp << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                 << "Content-Length: " << body.str().size() << "\r\n\r\n"
                 << body.str();
            send(client, resp.str().c_str(), resp.str().size(), 0);
        }
        else {
            const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client, not_found, strlen(not_found), 0);
        }

        close_socket(client);
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Usage: server udp_ip udp_port http_port\n";
        return -1;
    }

    const char* ip_str = argv[1];
    int udp_port = atoi(argv[2]);
    int http_port = atoi(argv[3]);

#if defined(WIN32) || defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    if (!serial_port_open(ip_str, udp_port)) {
        std::cout << "Failed to open data source!\n";
        return -1;
    }

    if (!std::filesystem::exists("data")) std::filesystem::create_directory("data");

    if (!db_init("data/temperature.db")) {
        std::cout << "DB init failed\n";
        return -1;
    }

    std::thread(http_server_thread, http_port).detach();

    char buf[256];
    std::cout << "Sensor server running...\n";

    while (true) {
        if (!serial_port_read_line(buf, sizeof(buf))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        double temp = atof(buf);
        db_insert_temperature(temp);
        std::cout << "Saved temp: " << temp << "\n";
    }
}
