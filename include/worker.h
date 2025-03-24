#ifndef WORKER_H
#define WORKER_H

#include <atomic>
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

public:
    Worker(const QueueableRegistry &registry_);
    ~Worker();
    // Prevent copying
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;
    
    void run();
    std::unique_ptr<Job> next_job(sqlite3 *db);
    void execute_job(Job &job);
    void cleanup_job(Job &job, bool succeeded=true);
};







#endif // WORKER_H
