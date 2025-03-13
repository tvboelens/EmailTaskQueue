#ifndef JOB_H
#define JOB_H

#include <string>
#include <nlohmann/json.hpp>
#include "randomhex.h"

using json = nlohmann::json;

class Job
{
public:
    std::string id;
    json args;

    Job(const json &args);
    void save() const;
    static Job fetch_next_job();

//private:
//    static std::string generate_random_id(size_t length);
};

#endif // JOB_H
