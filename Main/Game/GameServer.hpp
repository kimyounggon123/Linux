#ifndef GAMESERVER_H
#define GAMESERVER_H


#include "../../Core/Server/UDP/UDPServer.hpp"

#include "GameDispatcher.hpp"
#include "GameTaskWorker.hpp"
#include "AOI/AOIComponent.hpp"
class GameServer : public UDPServer
{
    GameDispatcher dispatcher;
    AOIComponent aoiComponent;
    bool MakeTaskWorkers() override;

public:
    GameServer(bool primateAddrFlag,  uint16_t port, size_t threadPoolCount): 
        UDPServer(AF_INET, primateAddrFlag, port, threadPoolCount),
        aoiComponent(threadPoolCount)
    {}

    void Start() override;
    void Stop() override;
};


#endif