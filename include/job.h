#ifndef JOB_H
#define JOB_H

#include <chrono>
#include <string>
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include "randomhex.h"

using json = nlohmann::json;

class Job
{
// Database column names as job attributes
private:
    std::string id;
    std::string name;                                                           // Name of Queueable subclass, which the job handles
    std::string queue;                                                          // Queue name (indicates priority)
    json args;
    int attempts;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> next_execution_at;
    std::optional<std::chrono::system_clock::time_point> last_executed_at;
    std::string state;
    std::optional<std::string> error_details;
    std::optional<std::string> reserved_by;                                     //id of worker which wants to execute this job

public:
    Job(const json &args_,
        const std::string &name_ = "Queueable",
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::chrono::system_clock::time_point> next_execution_at_ = std::nullopt,
        std::optional<std::chrono::system_clock::time_point> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    Job(const std::string &id_,
        const json &args_,
        const std::string &name_ = "Queueable",
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::chrono::system_clock::time_point> next_execution_at_ = std::nullopt,
        std::optional<std::chrono::system_clock::time_point> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    void save(sqlite3 *db = nullptr) const;
    std::string get_id() const;
    std::string get_name() const;
    json get_args() const;
    int get_attempts() const;
    std::optional<std::chrono::system_clock::time_point> get_last_attempt();
    void set_reserved_by(std::optional<std::string> worker_id);
    void increase_attempts();
    void set_latest_attempt_to_now();
    void set_state(const std::string &state_);
    void set_queue(const std::string &queue_);
    void set_error_details(std::optional<std::string> error_msg);
    void set_next_attempt(std::optional<std::chrono::system_clock::time_point> next_attempt);
};

#endif // JOB_H
