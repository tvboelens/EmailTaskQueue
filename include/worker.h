#ifndef WORKER_H
#define WORKER_H

#include <memory>
#include "job.h"
#include "randomhex.h"



class Worker
{
private:
    int polling_interval;
    std::string worker_id;

public:
    Worker();
    void run();
    std::unique_ptr<Job> next_job();
    //Job check_for_jobs();
    //void reserve_job(const Job &job);
    //void execute_job(const Job &job);
    //void cleanup_job(const Job &job);
};







#endif // WORKER_H
