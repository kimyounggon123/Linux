#ifndef IMANAGER_H
#define IMANAGER_H

#include "../../Utils/Thread/ThreadSafeContainor.hpp"

class IManager
{
public:
    IManager() = default;
    virtual ~IManager() = default;
    virtual void Process() = 0; // inf loop 금지!
};

#endif