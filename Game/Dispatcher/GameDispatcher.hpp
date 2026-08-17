#ifndef GAMEDISPATCHER_H
#define GAMEDISPATCHER_H

#include "../Data/GameData.hpp"
#include "../Entity/EntityManager.hpp"
#include "GameTask.hpp"

#include "../../Core/Utils/Log.hpp"
struct GameContext
{
    PlayerManager& playerManager;
    RoomManager& roomManager;

    GameContext(PlayerManager& playerManager, RoomManager& roomManager): playerManager(playerManager), roomManager(roomManager) {}
    GameContext(const GameContext& other): playerManager(other.playerManager), roomManager(other.roomManager) {}
};

class GameDispatcher
{
public:
    using Handler = GameTaskResult(GameDispatcher::*)(GameTask&, GameContext&);

private:
    static constexpr uint32_t MaxSize = ChangeToUINT(GameTaskType::LAST_DUMMY);
    bool isInitialized;
    std::array<Handler, MaxSize> handlers;

    GameTaskResult Move(GameTask& task, GameContext& ctx);
    GameTaskResult NULL_METHOD(GameTask& task,  GameContext& ctx) { return GameTaskResult::Fail; }
public:  
    GameDispatcher(): isInitialized(false)
    {
        handlers.fill(&GameDispatcher::NULL_METHOD);
    }
    ~GameDispatcher(){}
    
    bool Initialize();


    GameTaskResult Dispatch(uint32_t taskID, GameTask& task,  GameContext& ctx)
    {
        if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(task, ctx);
        return (this->*handlers[taskID])(task, ctx);
    }
};

#endif