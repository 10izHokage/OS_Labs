#include "HTTPClient.h"
#include <SFML/Network.hpp>
#include <sstream>
#include <iostream>

HTTPClient::HTTPClient(const std::string& h, int p)
    : host(h), port(p) {}

std::string HTTPClient::request(const std::string& path)
{
    sf::TcpSocket socket;

    auto ipOpt = sf::IpAddress::resolve(host);
    if (!ipOpt.has_value()) {
        std::cout << "Invalid host\n";
        return "";
    }

    if (socket.connect(*ipOpt, port) != sf::Socket::Status::Done) {
        std::cout << "Connection failed\n";
        return "";
    }

    std::ostringstream ss;
    ss << "GET " << path << " HTTP/1.1\r\n"
       << "Host: " << host << "\r\n"
       << "Connection: close\r\n\r\n";

    std::string req = ss.str();

    if (socket.send(req.c_str(), req.size()) != sf::Socket::Status::Done) {
        std::cout << "Send failed\n";
        return "";
    }

    std::string response;
    char buffer[2048];
    std::size_t received;

    while (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done)
        response.append(buffer, received);

    size_t bodyPos = response.find("\r\n\r\n");
    if (bodyPos == std::string::npos) return "";
    return response.substr(bodyPos + 4);
}

double HTTPClient::getCurrent()
{
    std::string body = request("/current");
    if (body.empty()) return 0;

    size_t pos = body.find(":");
    if (pos == std::string::npos) return 0;

    return std::stod(body.substr(pos + 1));
}

std::vector<TempPoint> HTTPClient::getHourly()
{
    std::string body = request("/hourly");
    std::vector<TempPoint> result;

    std::stringstream ss(body);
    std::string item;

    while (std::getline(ss, item, '{')) {
        size_t t = item.find("\"time\"");
        size_t v = item.find("\"avg\"");
        if (t == std::string::npos || v == std::string::npos) continue;

        std::string time = item.substr(item.find("\"", t + 7) + 1);
        time = time.substr(0, time.find("\""));

        double val = std::stod(item.substr(item.find(":", v) + 1));

        result.push_back({ time, val });
    }

    return result;
}

std::vector<TempPoint> HTTPClient::getDaily()
{
    std::string body = request("/daily");
    std::vector<TempPoint> result;

    std::stringstream ss(body);
    std::string item;

    while (std::getline(ss, item, '{')) {
        size_t t = item.find("\"date\"");
        size_t v = item.find("\"avg\"");
        if (t == std::string::npos || v == std::string::npos) continue;

        std::string date = item.substr(item.find("\"", t + 7) + 1);
        date = date.substr(0, date.find("\""));

        double val = std::stod(item.substr(item.find(":", v) + 1));

        result.push_back({ date, val });
    }

    return result;
}
