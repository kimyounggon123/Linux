#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include "../../Core/Server/Workers/TaskWorker.hpp" 
#include "../../Core/Server/NetworkTask.hpp"
#include "DatabaseDispatcher.hpp"

class DatabaseTaskWorker : public NetworkTaskProcessWorker<DatabaseDispatcher, DatabaseUtils>
{
public:
    DatabaseTaskWorker(const CoreServices& services,
        const DatabaseUtils& utils,
        DatabaseDispatcher& dispatcher,
        uint32_t ID = 0):
        NetworkTaskProcessWorker(services, utils, dispatcher, ID)
    {}
    ~DatabaseTaskWorker()
    {
        utils.Clear();
    }

    bool Initialize() override;
};

#endif