#ifndef DBPIPEPOOL_H
#define DBPIPEPOOL_H


#include "../Tool/RedisController.hpp"
#include "../Dispatcher/DatabaseTask.hpp"
#include "../../Core/Pipe/PipePool.hpp"
#include "../../Core/Utils/Containor/Containors.hpp"

using DBPipePool = PipePool<DatabaseTask>;
#endif