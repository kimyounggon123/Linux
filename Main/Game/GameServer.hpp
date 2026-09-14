#ifndef GAMESERVER_H
#define GAMESERVER_H


#include "../../Core/Server/UDP/UDPServer.hpp"

class GameServer : public UDPServer
{


public:
    GameServer(bool primateAddrFlag,  uint16_t port, size_t threadPoolCount): 
        UDPServer(AF_INET, primateAddrFlag, port, threadPoolCount)
    {}
};


#endif