#include "../include/queueable.h"

Queueable::Queueable(/* args */)
{
}

void Queueable::dispatch(const json &args){
    Job job{args};
    //spdlog::info("args={}", job->args);
    job.save();
    spdlog::info("Enqueued job id={}, args = ", job.get_id(), job.get_args().dump());
}

void Queueable::handle()
{
}

// QueueableRegistry class
QueueableRegistry::QueueableRegistry(/* args */)
{
}

QueueableRegistry::~QueueableRegistry()
{
}
// Register a new job class in the job registry
void QueueableRegistry::registerQueueable(const std::string &name, QueueableFactory factory)
{
    registry[name] = factory;
}
// Factory method. Make sure the class of the to be created object is registered first!
std::unique_ptr<Queueable> QueueableRegistry::createQueueable(const std::string &name) const
{
    // Error handling?
    return registry.at(name)();
}
