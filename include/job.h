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
    const unsigned char *next_execution_at;
    const unsigned char *last_executed_at;
    std::string state;
    const unsigned char *error_details;
    const unsigned char *reserved_by;
public:
    Job(const json &args_, 
         const std::string &queue_ = "default",
         const int &attempts_ = 0, 
         const unsigned char *next_execution_at_ = nullptr,
         const unsigned char *last_executed_at_ = nullptr,
         const std::string &state_ = "waiting",
         const unsigned char *error_details_ = nullptr,
         const unsigned char *reserved_by_ = nullptr);
    Job(const std::string &id_,
        const json &args_,
        const std::string &queue_ = "default",
        const int &attempts_ = 0,
        const unsigned char *next_execution_at_ = nullptr,
        const unsigned char *last_executed_at_ = nullptr,
        const std::string &state_ = "waiting",
        const unsigned char *error_details_ = nullptr,
        const unsigned char *reserved_by_ = nullptr);
    void save() const;
    std::string get_id() const;

    // private:
    //     static std::string generate_random_id(size_t length);
};

#endif // JOB_H
