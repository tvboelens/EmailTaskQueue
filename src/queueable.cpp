#include "../include/queueable.h"

Queueable::Queueable(/* args */)
{
}

void Queueable::dispatch(const json &args){
    Job job{args};
    spdlog::info("Enqueued job id={}", job.get_id());
    //spdlog::info("args={}", job->args);
    job.save();
}
