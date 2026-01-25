#include "db.h"
#include "sqlite3.h"
#include <iostream>
#include <sstream>

static sqlite3* g_db = nullptr;

bool db_init(const std::string& filename)
{
    if (sqlite3_open(filename.c_str(), &g_db) != SQLITE_OK) {
        std::cout << "DB open error\n";
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS measurements ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "value REAL,"
        "ts DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* err = nullptr;
    if (sqlite3_exec(g_db, sql, 0, 0, &err) != SQLITE_OK) {
        std::cout << "Table create error\n";
        sqlite3_free(err);
        return false;
    }

    return true;
}

void db_close()
{
    if (g_db)
        sqlite3_close(g_db);
}

bool db_insert_temperature(double value)
{
    std::string sql = "INSERT INTO measurements(value) VALUES(" + std::to_string(value) + ");";
    return sqlite3_exec(g_db, sql.c_str(), 0, 0, 0) == SQLITE_OK;
}

double db_get_last_temperature()
{
    double result = 0.0;
    const char* sql = "SELECT value FROM measurements ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW)
            result = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return result;
}

// Средние значения по минутам за последний час
std::vector<std::pair<std::string,double>> db_get_hourly_avg(int minutes)
{
    std::vector<std::pair<std::string,double>> result;
    std::ostringstream sql;

    sql << "SELECT strftime('%Y-%m-%d %H:%M', ts) AS tm, AVG(value) "
        << "FROM measurements "
        << "WHERE ts >= datetime('now','-" << minutes << " minutes') "
        << "GROUP BY tm ORDER BY tm;";
/*    sql << "SELECT strftime('%Y-%m-%d %H:%M:%S', ts) AS tm, AVG(value) "
        << "FROM measurements "
        << "WHERE ts >= datetime('now','-60 seconds') "
        << "GROUP BY tm ORDER BY tm;"; для теста вместо часа берем минуту*/ 

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string t = (const char*)sqlite3_column_text(stmt,0);
            double avg = sqlite3_column_double(stmt,1);
            result.emplace_back(t, avg);
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

// Средние значения по дням за последние 7 дней
std::vector<std::pair<std::string,double>> db_get_daily_avg(int days)
{
    std::vector<std::pair<std::string,double>> result;
    std::ostringstream sql;

    sql << "SELECT strftime('%Y-%m-%d', ts) AS tm, AVG(value) "
        << "FROM measurements "
        << "WHERE ts >= datetime('now','-" << days << " days') "
        << "GROUP BY tm ORDER BY tm;";
/*    sql << "SELECT strftime('%Y-%m-%d %H:%M', ts) AS tm, AVG(value) "
        << "FROM measurements "
        << "WHERE ts >= datetime('now','-7 minutes') "
        << "GROUP BY tm ORDER BY tm;"; берем 7 минут вместо 7 дней */
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string t = (const char*)sqlite3_column_text(stmt,0);
            double avg = sqlite3_column_double(stmt,1);
            result.emplace_back(t, avg);
        }
    }
    sqlite3_finalize(stmt);
    return result;
}