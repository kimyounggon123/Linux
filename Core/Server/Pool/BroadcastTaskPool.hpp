#ifndef BROADCASTTASKPOOL_H
#define BROADCASTTASKPOOL_H

#include "../../Utils/Thread/ThreadSafeContainor.hpp"
#include "../NetworkTask.hpp"

using BroadcastTaskPool = CachePool<BroadcastTask>;
#endif