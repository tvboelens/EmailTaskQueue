#include "../include/job.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <random>
#include <sqlite3.h>

// Constructor - Assigns a unique ID and sets args
Job::Job(const json &args)
{
    
    this->id = "job_" + generateHex(12);
    this->args = args;
}

// Generates a random hexadecimal string
/* std::string Job::generate_random_id(size_t length)
{
    static const char hex_chars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());                         // Mersenne Twister PRNG
    std::uniform_int_distribution<> distrib(0, 15); // Values from 0 to 15 (hex range)

    std::stringstream ss;
    for (size_t i = 0; i < length; ++i)
    {
        ss << hex_chars[distrib(gen)];
    }
    return ss.str();
}*/

void Job::save() const{
    sqlite3 *db;
    char *errMsg = nullptr;

    if (sqlite3_open("jobs.db", &db) != SQLITE_OK)
    {
        spdlog::error("Failed to open database.");
        return;
    }

    std::string sql = "INSERT INTO jobs (id, args, status) VALUES ('" + id + "', '" + args.dump() + "', 'pending');";

    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        spdlog::error("SQL error: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else
    {
        spdlog::info("Job saved to database: {}", id);
    }

    sqlite3_close(db);
}
