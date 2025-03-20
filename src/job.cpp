#include "../include/job.h"
#include "../include/chronotostring.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <random>

// Constructor - Assigns a unique ID and sets args
Job::Job(const json &args_,
         const std::string &name_,
         const std::string &queue_,
         const int &attempts_,
         std::optional<std::chrono::system_clock::time_point> next_execution_at_,
         std::optional<std::chrono::system_clock::time_point> last_executed_at_,
         const std::string &state_,
         std::optional<std::string> error_details_,
         std::optional<std::string> reserved_by_) : args{args_},
                                                    name{name_},
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
         const std::string &name_,
         const std::string &queue_,
         const int &attempts_,
         std::optional<std::chrono::system_clock::time_point> next_execution_at_,
         std::optional<std::chrono::system_clock::time_point> last_executed_at_,
         const std::string &state_,
         std::optional<std::string> error_details_,
         std::optional<std::string> reserved_by_) : id{id_},
                                                    args{args_},
                                                    name{name_},
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

void Job::save(sqlite3 *db) const
{
    sqlite3_stmt *stmt;
    char *errMsg = nullptr;
    bool connection_passed = true;
    // Connect to database.db if no connection is passed
    if (db==nullptr)
    {
        connection_passed = false;
        if (sqlite3_open("database.db", &db) != SQLITE_OK)
        {
            spdlog::error("Failed to open database. Cannot save job with job id = {}", id);
            return;
        }    
        
    }

    // SQL INSERT statement using prepared statements
    const char *sql = R"(
    INSERT INTO jobs (
        id, name, args, queue, created_at, next_execution_at, 
        last_executed_at, attempts, state, error_details, reserved_by
    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(id) DO UPDATE SET 
        name = excluded.name, 
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
        spdlog::error("Failed to prepare statement: {}, job id = {}", sqlite3_errmsg(db), id);
        sqlite3_close(db);
        return;
    }

    // Bind values to the placeholders
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);                           // id
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);                         // name
    std::string args_str = args.dump();                                                  // Serialize JSON to string
    sqlite3_bind_text(stmt, 3, args_str.c_str(), -1, SQLITE_STATIC);                     // args (JSON serialized to string)
    sqlite3_bind_text(stmt, 4, queue.c_str(), -1, SQLITE_STATIC);                        // queue
    sqlite3_bind_text(stmt, 5, chrono_to_string(created_at).c_str(), -1, SQLITE_STATIC); // created_at

    // Bind next_execution_at (nullable)
    if (next_execution_at)
    {
        sqlite3_bind_text(stmt, 6, chrono_to_string(*next_execution_at).c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 6); // NULL if no value
    }

    // Bind last_executed_at (nullable)
    if (last_executed_at)
    {
        sqlite3_bind_text(stmt, 7, chrono_to_string(*last_executed_at).c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 7); // NULL if no value
    }

    sqlite3_bind_int(stmt, 8, attempts);                          // attempts
    sqlite3_bind_text(stmt, 9, state.c_str(), -1, SQLITE_STATIC); // state

    // Bind error_details (nullable)
    if (error_details)
    {
        sqlite3_bind_text(stmt, 10, error_details->c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 10); // NULL if no value
    }

    // Bind reserved_by (nullable)
    if (reserved_by)
    {
        sqlite3_bind_text(stmt, 11, reserved_by->c_str(), -1, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_null(stmt, 11); // NULL if no value
    }

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        spdlog::error("Failed to insert job: {}, job id = {}", sqlite3_errmsg(db),id);
    }
    else
    {
        spdlog::info("Job saved to database: {}", id);
    }

    // Clean up
    sqlite3_finalize(stmt);
    // Only close database connection if it was opened by worker itself
    if (!connection_passed)
    {
        sqlite3_close(db);
    }
}

std::string Job::get_id() const
{
    return id;
}


std::string Job::get_name() const
{
    return name;
}

json Job::get_args() const
{
    return args;
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

void Job::set_queue(const std::string &queue_)
{
    queue = queue_;
}

void Job::set_error_details(std::optional<std::string> error_msg)
{
    if (error_msg.has_value())
    {
        error_details = std::move(error_msg);
    }
    else
    {
        error_details = std::nullopt;
    }
}

void Job::set_next_attempt(std::optional<std::chrono::system_clock::time_point> next_attempt)
{
    if (next_attempt.has_value())
    {
        next_execution_at = std::move(next_attempt);
    }
    else
    {
        next_execution_at = std::nullopt;
    }
}
