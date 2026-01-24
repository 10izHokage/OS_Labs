#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>     
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <time.h>

#include "serial_port.h"

static time_t now_time()
{
    return time(NULL);
}

static std::tm local_tm(time_t t)
{
    std::tm tmv{};
#if defined(WIN32) || defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&tmv, &t);
#endif
    return tmv;
}

static std::string format_datetime(time_t t)
{
    std::tm tmv = local_tm(t);
    char buf[32];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
            tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return std::string(buf);
}

static std::string format_hour_key(time_t t)
{
    std::tm tmv = local_tm(t);
    char buf[32];
    sprintf(buf, "%04d-%02d-%02d %02d:00",
            tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
            tmv.tm_hour);
    return std::string(buf);
}

static std::string format_date_key(time_t t)
{
    std::tm tmv = local_tm(t);
    char buf[16];
    sprintf(buf, "%04d-%02d-%02d",
            tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday);
    return std::string(buf);
}

static bool parse_measurement_line(const std::string& line, time_t& out_t)
{
    int Y,M,D,h,m,s;
    double temp;
    if (sscanf(line.c_str(), "%d-%d-%d %d:%d:%d %lf", &Y,&M,&D,&h,&m,&s,&temp) != 7)
        return false;

    std::tm tmv{};
    tmv.tm_year = Y - 1900;
    tmv.tm_mon  = M - 1;
    tmv.tm_mday = D;
    tmv.tm_hour = h;
    tmv.tm_min  = m;
    tmv.tm_sec  = s;
    tmv.tm_isdst = -1;

    out_t = mktime(&tmv);
    return true;
}

static bool parse_hourly_line(const std::string& line, time_t& out_t)
{
    int Y,M,D,h;
    double avg;
    if (sscanf(line.c_str(), "%d-%d-%d %d:00 %lf", &Y,&M,&D,&h,&avg) != 5)
        return false;

    std::tm tmv{};
    tmv.tm_year = Y - 1900;
    tmv.tm_mon  = M - 1;
    tmv.tm_mday = D;
    tmv.tm_hour = h;
    tmv.tm_min  = 0;
    tmv.tm_sec  = 0;
    tmv.tm_isdst = -1;

    out_t = mktime(&tmv);
    return true;
}

static bool parse_daily_line(const std::string& line, int& out_year)
{
    int Y,M,D;
    double avg;
    if (sscanf(line.c_str(), "%d-%d-%d %lf", &Y,&M,&D,&avg) != 4)
        return false;
    out_year = Y;
    return true;
}

static void append_line(const std::string& path, const std::string& line)
{
    std::ofstream out(path, std::ios::app);
    if (out.is_open())
        out << line << "\n";
}


static void trim_measurements_24h(const std::string& path, time_t now)
{
    const time_t cutoff = now - 24 * 60 * 60; // 24 часа
    // const time_t cutoff = now - 120;  // 2 минуты для ускоренного теста

    std::ifstream in(path);
    if (!in.is_open()) return;

    std::vector<std::string> keep;
    std::string line;
    while (std::getline(in, line)) {
        time_t t;
        if (parse_measurement_line(line, t) && t >= cutoff)
            keep.push_back(line);
    }
    in.close();

    std::ofstream out(path, std::ios::trunc);
    for (auto& s : keep) out << s << "\n";
}

static void trim_hourly_1month(const std::string& path, time_t now)
{
    const time_t cutoff = now - 30 * 24 * 60 * 60;  // 30 дней (месяц)
    // const time_t cutoff = now - 120;  // 2 минуты для ускоренного теста

    std::ifstream in(path);
    if (!in.is_open()) return;

    std::vector<std::string> keep;
    std::string line;
    while (std::getline(in, line)) {
        time_t t;
        if (parse_hourly_line(line, t) && t >= cutoff)
            keep.push_back(line);
    }
    in.close();

    std::ofstream out(path, std::ios::trunc);
    for (auto& s : keep) out << s << "\n";
}

static void trim_daily_current_year(const std::string& path, int current_year)
{
    std::ifstream in(path);
    if (!in.is_open()) return;

    std::vector<std::string> keep;
    std::string line;
    while (std::getline(in, line)) {
        int y;
        if (parse_daily_line(line, y) && y == current_year)
            keep.push_back(line);
    }
    in.close();

    std::ofstream out(path, std::ios::trunc);
    for (auto& s : keep) out << s << "\n";
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: server ip port\n";
        return -1;
    }

    const char* ip_str = argv[1];
    int port = atoi(argv[2]);

    if (!serial_port_open(ip_str, port)) {
        std::cout << "Failed to open source!\n";
        return -1;
    }

    const std::string MEAS_LOG = "data/measurements.log";
    const std::string HOUR_LOG = "data/hourly_avg.log";
    const std::string DAY_LOG  = "data/daily_avg.log";

    std::cout << "Temp monitor started.\n";
    std::cout << "Listening on " << ip_str << ":" << port << "\n";

    time_t tstart = now_time();
    std::string current_hour_key = format_hour_key(tstart);
    std::string current_day_key  = format_date_key(tstart);

    double hour_sum = 0.0;
    int hour_cnt = 0;

    double day_sum = 0.0;
    int day_cnt = 0;

    time_t last_trim = 0;

    char buf[256];
    memset(buf, 0, sizeof(buf));

    for (;;) {
        if (!serial_port_read_line(buf, sizeof(buf))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        double temp = 0.0;
        if (sscanf(buf, "%lf", &temp) != 1) {
            std::cout << "Bad data: '" << buf << "'\n";
            continue;
        }

        time_t tnow = now_time();

        append_line(MEAS_LOG, format_datetime(tnow) + " " + std::to_string(temp));

        std::string hour_key = format_hour_key(tnow);
        std::string day_key  = format_date_key(tnow);

        if (hour_key != current_hour_key) {
            if (hour_cnt > 0) {
                double avg = hour_sum / (double)hour_cnt;
                append_line(HOUR_LOG, current_hour_key + " " + std::to_string(avg));
                trim_hourly_1month(HOUR_LOG, tnow);
            }
            current_hour_key = hour_key;
            hour_sum = 0.0;
            hour_cnt = 0;
        }

        if (day_key != current_day_key) {
            if (day_cnt > 0) {
                double avg = day_sum / (double)day_cnt;
                append_line(DAY_LOG, current_day_key + " " + std::to_string(avg));

                int cyear = local_tm(tnow).tm_year + 1900;
                trim_daily_current_year(DAY_LOG, cyear);
            }
            current_day_key = day_key;
            day_sum = 0.0;
            day_cnt = 0;
        }

        hour_sum += temp;
        hour_cnt++;
        day_sum += temp;
        day_cnt++;

        if (last_trim == 0 || (tnow - last_trim) >= 60) {
            trim_measurements_24h(MEAS_LOG, tnow);
            last_trim = tnow;
        }
    }

    serial_port_close();
    return 0;
}
