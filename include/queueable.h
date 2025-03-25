#ifndef QUEUEABLE_H
#define QUEUEABLE_H

#include <chrono>
#include <spdlog/spdlog.h>
#include "./job.h"

class Queueable
{
private:
public:
    Queueable();
    virtual void dispatch(const json &args, const std::string &name = "Queueable",
                          double wait = 0, std::optional<std::chrono::system_clock::time_point> at = std::nullopt);
    virtual void handle(const json &args, std::optional<json> credentials = std::nullopt);
};

// QueableFactory and QueueableRegistry provide a way to register subclasses of the Queueable class
// and then create instances of these via their name
using QueueableFactory = std::function<std::unique_ptr<Queueable>(void)>;

class QueueableRegistry
{
private:
    std::unordered_map<std::string, QueueableFactory> registry;

public:
    QueueableRegistry(/* args */);
    ~QueueableRegistry();
    void registerQueueable(const std::string &name, QueueableFactory factory);
    std::unique_ptr<Queueable> createQueueable(const std::string &name) const;
};

// Dummy class for experimenting
class LogQueueable: public Queueable
{
private:
    //std::string log_msg;

public:
    LogQueueable();
    // LogQueueable(const std::string &log_msg);
    ~LogQueueable();
    void handle(const json &args, std::optional<json> credentials = std::nullopt) override;
    void dispatch(const json &args);
};

#endif // QUEUEABLE_H
