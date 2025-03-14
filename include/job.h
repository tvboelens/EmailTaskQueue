#ifndef JOB_H
#define JOB_H

#include <string>
#include <nlohmann/json.hpp>
#include "randomhex.h"

using json = nlohmann::json;

class Job
{
// TODO: Set id and args to private and implement getter methods
public:
    std::string id;
    json args;

    Job(const json &args_);
    Job(const std::string &id_, const json &args_);
    void save() const;

//private:
//    static std::string generate_random_id(size_t length);
};

#endif // JOB_H
