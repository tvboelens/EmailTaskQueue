#include "../include/queueable.h"

// Queueable class
Queueable::Queueable(/* args */)
{
}
// TODO: I think dispatch needs to have the name as variable as well
void Queueable::dispatch(const json &args, const std::string &name){
    Job job{args, name};
    job.save();
    spdlog::info("Enqueued job id={}, args = {}, name = {}", job.get_id(), job.get_args().dump(),name);
}

void Queueable::handle(const json &args)
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

// LogQueueable class
LogQueueable::LogQueueable()
{
}
/*
LogQueueable::LogQueueable(const std::string &log_msg_): log_msg{log_msg_}
{
}
*/

LogQueueable::~LogQueueable()
{
}

void LogQueueable::dispatch(const json &args)
{
    Queueable::dispatch(args, "LogQueueable");
}

void LogQueueable::handle(const json& args)
{
    sleep(2);
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    if (r >= 0.6)
    {
        throw std::runtime_error("An error occurred.");
    }
    spdlog::info("LogQueueable #{}", args.dump());
}
