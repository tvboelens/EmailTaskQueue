#include "../include/job.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <random>
#include <sqlite3.h>

// Constructor - Assigns a unique ID and sets args
Job::Job(const json &args_,
         const std::string &queue_,
         const int &attempts_,
         std::optional<std::string> next_execution_at_,
         std::optional<std::string> last_executed_at_,
         const std::string &state_,
         std::optional<std::string> error_details_,
         std::optional<std::string> reserved_by_) : args{args_},
                                                    name{"log"},
                                                    queue{queue_},
                                                    attempts{attempts_},
                                                    next_execution_at{next_execution_at_},
                                                    last_executed_at{last_executed_at_},
                                                    state{state_},
                                                    error_details{error_details_},
                                                    reserved_by{reserved_by_}
{
    created_at = std::chrono::system_clock::now();
    id = "job_" + generateHex(12);
}

Job::Job(const std::string &id_,
         const json &args_,
         const std::string &queue_,
         const int &attempts_,
         std::optional<std::string> next_execution_at_,
         std::optional<std::string> last_executed_at_,
         const std::string &state_,
         std::optional<std::string> error_details_,
         std::optional<std::string> reserved_by_) : id{id_},
                                                                   args{args_},
                                                                   name{"log"},
                                                                   queue{queue_},
                                                                   attempts{attempts_},
                                                                   next_execution_at{next_execution_at_},
                                                                   last_executed_at{last_executed_at_},
                                                                   state{state_},
                                                                   error_details{error_details_},
                                                                   reserved_by{reserved_by_}
{
    created_at = std::chrono::system_clock::now();
}


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

std::string Job::get_id() const
{
    return id;
}
