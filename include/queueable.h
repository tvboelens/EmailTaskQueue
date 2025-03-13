#ifndef QUEUEABLE_H
#define QUEUEABLE_H

#include <spdlog/spdlog.h>
#include "./job.h"

class Queueable
{
private:
public:
    Queueable();
    virtual void dispatch(const json &args); // TODO: Do I need to put in options?
};





#endif // QUEUEABLE_H
