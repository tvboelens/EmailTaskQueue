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
// TODO: Are getter methods necessary and if so, which ones?
private:
    std::string id;
    std::string name;
    std::string queue;
    json args;
    int attempts;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> next_execution_at;
    std::optional<std::chrono::system_clock::time_point> last_executed_at;
    std::string state;
    std::optional<std::string> error_details;
    std::optional<std::string> reserved_by;

public:
    Job(const json &args_,
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::chrono::system_clock::time_point> next_execution_at_ = std::nullopt,
        std::optional<std::chrono::system_clock::time_point> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    Job(const std::string &id_,
        const json &args_,
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        std::optional<std::chrono::system_clock::time_point> next_execution_at_ = std::nullopt,
        std::optional<std::chrono::system_clock::time_point> last_executed_at_ = std::nullopt,
        const std::string &state_ = "waiting",
        std::optional<std::string> error_details_ = std::nullopt,
        std::optional<std::string> reserved_by_ = std::nullopt);
    void save(sqlite3 *db = nullptr) const;
    std::string get_id() const;
    void set_reserved_by(std::optional<std::string> worker_id);
    void increase_attempts();
    void set_latest_attempt_to_now();
    void set_state(const std::string &state_);
};

#endif // JOB_H
