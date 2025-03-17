#include "../include/worker.h"

#include <chrono>
#include <spdlog/spdlog.h>
#include <unistd.h>

Worker::Worker()
{
    polling_interval = 5; // Check for new jobs every 5s
    worker_id = "wrk_" + generateHex(8);
}

Worker::~Worker()
{
    // Close the database connection if an error occurs or if the worker is destroyed
    if (db)
    {
        sqlite3_close_v2(db);
        db = nullptr;
        spdlog::info("Shut down database connection: {}", worker_id);
    }
}


void Worker::run()
{
    spdlog::info("Worker {} ready", worker_id);
    if (sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        spdlog::error("Failed to open database.");
        return;
    }
    else{
        spdlog::info("Established database connection: worker {}", worker_id);
    }
    int counter = 0;
    do
    {
        std::unique_ptr<Job> job = next_job(db);
        if (job!=0)
        {
            spdlog::info("Executing job: {}", job->get_id());
            // execute_job(*job);
            cleanup_job(*job);
            counter += 1;
        }
        else
        {
            sleep(polling_interval);
            counter += 1;
        }
    } while (counter<7);
    spdlog::info("Worker {} shutting down connection to database", worker_id);
    spdlog::info("Shutting down Worker {}", worker_id);
}

std::unique_ptr<Job> Worker::next_job(sqlite3 *db){
    
    sqlite3_stmt *stmt;
    const char *sql = R"(
        UPDATE Jobs 
        SET reserved_by = ? 
        WHERE id = (
            SELECT id FROM jobs 
            WHERE reserved_by IS NULL AND state = 'waiting'
            LIMIT 1
        )
        RETURNING id, args, reserved_by, state;
    )";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        // Bind worker_id as a TEXT value
        sqlite3_bind_text(stmt, 1, worker_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string args_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            json args = nlohmann::json::parse(args_str);
            std::unique_ptr<Job> job{new Job{id, args}};
            spdlog::info("Fetched next job: {}", job->get_id());
            sqlite3_finalize(stmt);
            return job;
        }
        else
        {
            spdlog::warn("No pending jobs found.");
            sqlite3_finalize(stmt);
            return nullptr;
        }
    }
    else
    {
        spdlog::error("Failed to fetch job from database.");
        return nullptr;
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

void Worker::cleanup_job(Job &job)
{
    job.set_reserved_by(std::nullopt);
    job.increase_attempts();
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
    job.set_latest_attempt_to_now();
    job.set_state("succeeded");
    spdlog::info("Done cleaning up job {}, saving...", job.get_id());
    job.save();
    spdlog::info("Saved job: {}", job.get_id());
}
