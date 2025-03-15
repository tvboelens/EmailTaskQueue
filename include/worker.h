#ifndef WORKER_H
#define WORKER_H

#include <memory>
#include <sqlite3.h>
#include "job.h"
#include "randomhex.h"



class Worker
{
private:
    int polling_interval;
    std::string worker_id;
    sqlite3 *db;

public:
    Worker();
    ~Worker();
    // Prevent copying
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;
    
    void run();
    std::unique_ptr<Job> next_job(sqlite3 *db);
    //Job check_for_jobs();
    //void reserve_job(const Job &job);
    //void execute_job(const Job &job);
    void cleanup_job(const Job &job);
};







#endif // WORKER_H
