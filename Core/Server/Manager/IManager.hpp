#ifndef IMANAGER_H
#define IMANAGER_H

#include "../../Utils/Thread/ThreadPool.hpp"
#include "../../Utils/Containor/ThreadSafeContainor.hpp"
#include "../../Utils/Log.hpp"

class IManager
{
public:
    IManager() = default;
    virtual ~IManager() = default;
    virtual void Process() = 0; // inf loop 금지!
};

#endif