#include "../include/job.h"
#include "../include/chronotostring.h"
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
         std::optional<std::chrono::system_clock::time_point> next_execution_at_,
         std::optional<std::chrono::system_clock::time_point> last_executed_at_,
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
         std::optional<std::chrono::system_clock::time_point> next_execution_at_,
         std::optional<std::chrono::system_clock::time_point> last_executed_at_,
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

// TODO: Pass db connection to save() instead of establishing own connection.
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
    const char *sql = R"(
    INSERT INTO jobs (
        id, name, args, queue, created_at, next_execution_at, 
        last_executed_at, attempts, state, error_details, reserved_by
    ) VALUES (?, 'Job', ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(id) DO UPDATE SET 
        name = 'Job', 
        args = excluded.args,
        queue = excluded.queue,
        created_at = excluded.created_at,
        next_execution_at = excluded.next_execution_at,
        last_executed_at = excluded.last_executed_at,
        attempts = excluded.attempts,
        state = excluded.state,
        error_details = excluded.error_details,
        reserved_by = excluded.reserved_by;
)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind values to the placeholders
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);                           // id
    std::string args_str = args.dump();                                                  // Serialize JSON to string
    sqlite3_bind_text(stmt, 2, args_str.c_str(), -1, SQLITE_STATIC);                     // args (JSON serialized to string)
    sqlite3_bind_text(stmt, 3, queue.c_str(), -1, SQLITE_STATIC);                        // queue
    sqlite3_bind_text(stmt, 4, chrono_to_string(created_at).c_str(), -1, SQLITE_STATIC); // created_at

    // Bind next_execution_at (nullable)
    if (next_execution_at)
    {
        sqlite3_bind_text(stmt, 5, chrono_to_string(*next_execution_at).c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 5); // NULL if no value
    }

    // Bind last_executed_at (nullable)
    if (last_executed_at)
    {
        sqlite3_bind_text(stmt, 6, chrono_to_string(*last_executed_at).c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 6); // NULL if no value
    }

    sqlite3_bind_int(stmt, 7, attempts);                          // attempts
    sqlite3_bind_text(stmt, 8, state.c_str(), -1, SQLITE_STATIC); // state

    // Bind error_details (nullable)
    if (error_details)
    {
        sqlite3_bind_text(stmt, 9, error_details->c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 9); // NULL if no value
    }

    // Bind reserved_by (nullable)
    if (reserved_by)
    {
        sqlite3_bind_text(stmt, 10, reserved_by->c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 10); // NULL if no value
    }

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

void Job::set_reserved_by(std::optional<std::string> worker_id)
{
    reserved_by = std::move(worker_id);
}

void Job::increase_attempts()
{
    attempts += 1;
}

void Job::set_latest_attempt_to_now()
{
    last_executed_at = std::chrono::system_clock::now();
}

void Job::set_state(const std::string &state_)
{
    state = state_;
    //TODO: check if values are allowed and throw error if needed
}
