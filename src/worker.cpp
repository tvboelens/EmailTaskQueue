#include "../include/worker.h"

#include <chrono>
#include <spdlog/spdlog.h>
#include <unistd.h>

// Atomic flag to stop workers gracefully
std::atomic<bool> stopWorkers{false};

Worker::Worker(const QueueableRegistry &registry_, std::optional<json> credentials) : registry{&registry_}, smtp_credentials{credentials}
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
        spdlog::info("Shut down database connection: Worker {}", worker_id);
    }
}


void Worker::run()
{
    spdlog::info("Worker {} ready", worker_id);
    if (sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        spdlog::error("Failed to open database. Worker id = {}", worker_id);
        return;
    }
    else{
        spdlog::info("Established database connection: worker {}", worker_id);
    }

    do
    {
        std::unique_ptr<Job> job = next_job(db);
        if (job != 0)
        {
            spdlog::info("Worker {} executing job: {} with name = {}", worker_id, job->get_id(), job->get_name());
            execute_job(*job);
        }
        else
        {
            sleep(polling_interval);
        }
    } while (!stopWorkers);
    spdlog::info("Shutting down Worker {}", worker_id);
}

std::unique_ptr<Job> Worker::next_job(sqlite3 *db){
    // SQL statement to retrieve job from database
    sqlite3_stmt *stmt;
    const char *sql = R"(
        UPDATE Jobs 
        SET reserved_by = ? 
        WHERE id = (
            SELECT id FROM jobs 
            WHERE reserved_by IS NULL 
            AND (state = 'waiting' OR state = 'failed')
            AND (next_execution_at IS NULL OR next_execution_at <= ?)
            ORDER BY created_at ASC         -- Retrieve job which was created first
            LIMIT 1
        )
        RETURNING id, args, name, queue, attempts;
    )";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        // Bind worker_id and current timestamp as a TEXT value
        sqlite3_bind_text(stmt, 1, worker_id.c_str(), -1, SQLITE_STATIC);
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm *localTime = std::localtime(&now_c);

        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S"); // SQL datetime format
        std::string timestamp = oss.str();
        sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string args_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            json args = nlohmann::json::parse(args_str);
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            std::string queue = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            int attempts = sqlite3_column_int(stmt, 4);
            std::unique_ptr<Job> job{new Job{
                id,
                args,
                name,
                queue,
                attempts
            }};
            spdlog::info("Worker {}. Fetched next job: {}", worker_id, job->get_id());
            sqlite3_finalize(stmt);
            return job;
        }
        else
        {
            spdlog::warn("Worker {}. No pending jobs found.", worker_id);
            sqlite3_finalize(stmt);
            return nullptr;
        }
    }
    else
    {
        spdlog::error("Worker {}. Failed to fetch job from database.", worker_id);
        return nullptr;
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

void Worker::execute_job(Job &job)
{
    std::string name{job.get_name()};
    // Create the object which performs the task and handle possible exceptions that are thrown during creation
    try
    {
        std::unique_ptr<Queueable> q = registry->createQueueable(name);
        if (name=="SendEmail")
        {
            spdlog::info("Worker {} sending email", worker_id);
            q->handle(job.get_args(), smtp_credentials);
        }
        else
        {
            spdlog::info("Executing some other type of job");
            q->handle(job.get_args()); // Execute task
        }
        spdlog::info("Worker {}. Processed job id = {}, result = succeeded, name = {}", worker_id, job.get_id(), job.get_name());
        cleanup_job(job);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::error("Worker {} could not execute job with id = {}, class name = {} not registered", worker_id, job.get_id(), name);
        std::string error_msg = "Could not create class with name " + name + " since it was not registered ";
        job.set_error_details(error_msg);
        cleanup_job(job, false);
        return;
    }
    catch (const std::bad_function_call &e)
    {
        spdlog::error("Worker {} could not execute job with id = {}, factory for class with name = {} not properly defined", worker_id, job.get_id(), name);
        std::string error_msg = "Could not create class with name " + name + " since its factory was not properly defined";
        job.set_error_details(error_msg);
        cleanup_job(job, false);
        return;
    }
    catch(...)
    {
        std::exception_ptr exPtr = std::current_exception();
        try 
        {
            if (exPtr)
            {
                std::rethrow_exception(exPtr);
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Worker {} could not execute job with id = {}, caught exception with message '{}'", worker_id, job.get_id(), e.what());
            std::string error_msg = "Could not create class with name " + name + ", caught exception of type " + e.what();
            job.set_error_details(error_msg);
            cleanup_job(job, false);
            return;
        }
        catch (...)
        {
            spdlog::error("Worker {} could not execute job with id = {}, caught exception of unknown type", job.get_id());
            std::string error_msg = "Could not create class with name " + name + ", caught exception of unknown type.";
            job.set_error_details(error_msg);
            cleanup_job(job, false);
            return;
        }
    }
}

void Worker::cleanup_job(Job &job, bool succeeded)
{
    job.set_reserved_by(std::nullopt); 
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
    job.set_latest_attempt_to_now();
    if (succeeded){
        job.set_state("succeeded");
    }
    else
    {
        job.increase_attempts();
        job.set_state("failed");
        set_retry_details(registry->getRetryStrategy(job.get_name()), job);
    }
    spdlog::info("Worker {}. Done cleaning up job {} with name = {}, saving...", worker_id, job.get_id(), job.get_name());
    job.save(db);
}

void Worker::set_retry_details(const RetryStrategy &strategy,
                               Job &job)
{
    std::optional<double> interval;
    if (strategy.interval.has_value())
    {
        interval.emplace(strategy.interval.value());
    }
    else
    {
        interval = std::nullopt;
    }

    std::string queue = strategy.queue.has_value() ? strategy.queue.value() : "default"; 
    int max_retries = strategy.max_retries.has_value() ? strategy.max_retries.value() : -1;

    int retry_count = job.get_attempts() - 1;
    // Check if max retries exceeded
    if (max_retries != -1 && retry_count +1 >= max_retries)
    {
        job.set_state("dead");
        return;
    }

    if (strategy.block)
    {
        std::optional<double> decision{strategy.block(retry_count)};
        if (!decision)
        {
            job.set_state("dead");
            return;
        }
        interval = std::move(decision);
    }

    // Exponential backoff calculation
    if (interval.has_value() && interval.value() == -1)
    { // Use -1 as a placeholder for exponential
        interval.emplace(30 + std::pow(retry_count, 5));
    }

    if(job.get_last_attempt().has_value())
    {
        job.set_next_attempt(job.get_last_attempt().value() +
                std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(interval.value())));
    }
    else
    {
        job.set_next_attempt(std::chrono::system_clock::now() + 
            std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(interval.value())));
    }

    if (!queue.empty())
    {
        job.set_queue(queue);
    }
}
