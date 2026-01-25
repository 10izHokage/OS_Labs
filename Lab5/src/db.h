#pragma once
#include <string>
#include <vector>
#include <utility>

bool db_init(const std::string& filename);
void db_close();

bool db_insert_temperature(double value);
double db_get_last_temperature();

std::vector<std::pair<std::string,double>> db_get_hourly_avg(int minutes = 60);
std::vector<std::pair<std::string,double>> db_get_daily_avg(int days = 7);
