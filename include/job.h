#ifndef JOB_H
#define JOB_H

#include <chrono>
#include <string>
#include <nlohmann/json.hpp>
#include "randomhex.h"

using json = nlohmann::json;

class Job
{
// Database column names as job attributes
// TODO: Are getter methods necessary?
private:
    std::string id;
    std::string name;
    std::string queue;
    json args;
    int attempts;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::string> next_execution_at;
    std::optional<std::string> last_executed_at;
    std::string state;
    std::optional<std::string> error_details;
    std::optional<std::string> reserved_by;

public:
    Job(const json &args_,
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::string> next_execution_at_ = std::nullopt,
        std::optional<std::string> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    Job(const std::string &id_,
        const json &args_,
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::string> next_execution_at_ = std::nullopt,
        std::optional<std::string> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    void save() const;
    std::string get_id() const;
    void set_reserved_by(std::optional<std::string> worker_id);
    void increase_attempts();
    void set_latest_attempt(const std::chrono::system_clock::time_point &time);
    void set_state(const std::string &state_);

    // private:
    //     static std::string generate_random_id(size_t length);
};

#endif // JOB_H
