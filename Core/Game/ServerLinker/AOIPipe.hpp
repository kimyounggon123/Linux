#ifndef AOIPIPE_H
#define AOIPIPE_H

#include "../../Core/Pipe/PipePool.hpp"
#include "../Dispatcher/GameTask.hpp"

using AOIPipePool = PipePool<GameTask>;
#endif