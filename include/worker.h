#ifndef WORKER_H
#define WORKER_H

#include <atomic>
#include <optional>
#include <memory>
#include <sqlite3.h>
#include "job.h"
#include "randomhex.h"
#include "queueable.h"

// Atomic flag to stop workers gracefully
extern std::atomic<bool> stopWorkers;

class Worker
{
private:
    int polling_interval;
    std::string worker_id;
    sqlite3 *db;
    const QueueableRegistry *registry;
    std::optional<json> smtp_credentials;

public:
    Worker(const QueueableRegistry &registry_, std::optional<json> credentials = std::nullopt);
    ~Worker();
    // Prevent copying
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;
    
    void run();
    std::unique_ptr<Job> next_job(sqlite3 *db);
    void execute_job(Job &job);
    void cleanup_job(Job &job, bool succeeded = true);
    void set_retry_details(const RetryStrategy& retry_strategy,
                       Job& job);
};

#endif // WORKER_H
