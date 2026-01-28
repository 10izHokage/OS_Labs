#pragma once
#include <vector>
#include <string>
#include "TemperatureGraph.h"

class HTTPClient {
public:
    HTTPClient(const std::string& host, int port);

    double getCurrent();
    std::vector<TempPoint> getHourly();
    std::vector<TempPoint> getDaily();

private:
    std::string host;
    int port;

    std::string request(const std::string& path);
};


