#include "../include/queueable.h"

// Queueable class
Queueable::Queueable(/* args */)
{
}

void Queueable::dispatch(const json &args, const std::string &name,
                         double wait, std::optional<std::chrono::system_clock::time_point> at)
{
    Job job{args, name};
    if (wait != 0)
    {
        std::chrono::system_clock::time_point next_execution{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(wait)) + 
            std::chrono::system_clock::now()};
        job.set_next_attempt(next_execution);
    }
    else if (at.has_value())
    {
        std::chrono::system_clock::time_point next_execution = at.value();
        job.set_next_attempt(next_execution);
    }
    job.save();
    spdlog::info("Enqueued job id={}, args = {}, name = {}", job.get_id(), job.get_args().dump(),name);
}

void Queueable::handle(const json &args, std::optional<json> credentials)
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

void QueueableRegistry::registerRetryStrategy(const std::string &name, const RetryStrategy &strategy)
{
    retry_strategy_registry[name] = strategy;
}

// Factory method. Make sure the class of the to be created object is registered first!
std::unique_ptr<Queueable> QueueableRegistry::createQueueable(const std::string &name) const
{    
    return registry.at(name)();
}

RetryStrategy QueueableRegistry::getRetryStrategy(const std::string &name) const
{
    return retry_strategy_registry.at(name);
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

void LogQueueable::handle(const json& args, std::optional<json> credentials)
{
    sleep(2);
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    //if (r >= 0.4)
    throw std::runtime_error("An error occurred.");
    spdlog::info("LogQueueable #{}", args.dump());
}
