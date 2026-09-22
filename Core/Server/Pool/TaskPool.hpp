#ifndef BROADCASTTASKPOOL_H
#define BROADCASTTASKPOOL_H

#include "../../Utils/Containor/ThreadSafeContainor.hpp"
#include "../NetworkTask.hpp"

using BroadcastTaskPool = CachePool<BroadcastTask>;
using NetworkTaskPool = CachePool<NetworkTask>;
#endif