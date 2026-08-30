#ifndef SIGNWORKER_H
#define SIGNWORKER_H

#include "../../Core/Server/Workers/TaskWorker.hpp" 
#include "../../Core/Server/NetworkTask.hpp"
#include "SignDispatcher.hpp"


class SignTaskWorker : public NetworkTaskProcessWorker<SignDispatcher, SignUtilEx>
{
public:
    SignTaskWorker(const CoreServices& services, const SignUtilEx& utils, 
        SignDispatcher& dispatcher,
        uint32_t ID = 0):
        NetworkTaskProcessWorker(services, utils, dispatcher, ID)
    {}
};

#endif