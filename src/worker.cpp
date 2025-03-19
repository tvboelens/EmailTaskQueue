#include "../include/worker.h"

#include <chrono>
#include <spdlog/spdlog.h>
#include <unistd.h>

Worker::Worker(const QueueableRegistry &registry_): registry{&registry_}
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
            execute_job(*job);
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
            WHERE reserved_by IS NULL 
            AND state = 'waiting'
            AND (next_execution_at IS NULL OR next_execution_at <= CURRENT_TIMESTAMP)
            ORDER BY created_at ASC         -- Retrieve job which was created first
            LIMIT 1
        )
        RETURNING id, args, name, state;
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
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
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

void Worker::execute_job(Job &job)
{
    std::string name{job.get_name()};
    try
    {
        std::unique_ptr<Queueable> q = registry->createQueueable(name);
        q->handle();
        spdlog::info("Processed job id = {}, result = succeeded, args = {}", job.get_id(), job.get_args().dump());
        cleanup_job(job);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::error("Could not execute job with id = {}, class name = {} not registered", job.get_id(), name);
        std::string error_msg = "Could not create class with name " + name + " since it was not registered ";
        job.set_error_details(error_msg);
        cleanup_job(job, false);
        return;
    }
    catch (const std::bad_function_call &e)
    {
        spdlog::error("Could not execute job with id = {}, factory for class with name = {} not properly defined", job.get_id(), name);
        std::string error_msg = "Could not create class with name " + name + " since its factory was not properly defined";
        job.set_error_details(error_msg);
        cleanup_job(job, false);
        return;
    }
}

void Worker::cleanup_job(Job &job, bool succeeded)
{
    job.set_reserved_by(std::nullopt);
    job.increase_attempts();
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
    job.set_latest_attempt_to_now();
    if (succeeded){
        job.set_state("succeeded");
    }
    else
    {
        job.set_state("failed");
    }
    spdlog::info("Done cleaning up job {} with name = {}, saving...", job.get_id(), job.get_name());
    job.save(db);
}
