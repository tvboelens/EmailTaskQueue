#include "../include/queueable.h"

// Queueable class
Queueable::Queueable(/* args */)
{
}
// TODO: I think dispatch needs to have the name as variable as well
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
// Register a new class in the registry
void QueueableRegistry::registerQueueable(const std::string &name, QueueableFactory factory)
{
    registry[name] = factory;
}
// Factory method. Make sure the class of the to be created object is registered first!
std::unique_ptr<Queueable> QueueableRegistry::createQueueable(const std::string &name) const
{    
    return registry.at(name)();
}
