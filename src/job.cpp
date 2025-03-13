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
    sqlite3_stmt *stmt;
    char *errMsg = nullptr;

    if (sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        spdlog::error("Failed to open database.");
        return;
    }

    // SQL INSERT statement using prepared statements
    const char *sql = "INSERT INTO jobs (name, id, args, state) VALUES ('Job', ?, ?, 'waiting');";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind values safely
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    std::string args_str = args.dump();
    sqlite3_bind_text(stmt, 2, args_str.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        spdlog::error("Failed to insert job: {}", sqlite3_errmsg(db));
    }
    else
    {
        spdlog::info("Job saved to database: {}", id);
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
