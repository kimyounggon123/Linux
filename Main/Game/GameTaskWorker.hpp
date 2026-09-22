#ifndef GAMETASKWORKER_H
#define GAMETASKWORKER_H


#include "../../Core/Server/Workers/TaskWorker.hpp" 
#include "../../Core/Server/NetworkTask.hpp"
#include "GameDispatcher.hpp"

class GameTaskWorker : public NetworkTaskProcessWorker<GameDispatcher, GameUtils>
{
public:
    GameTaskWorker(const CoreServices& services, const GameUtils& utils, GameDispatcher& dispatcher, uint32_t ID = 0):
        NetworkTaskProcessWorker(services, utils, dispatcher, ID)
    {}
};

#endif