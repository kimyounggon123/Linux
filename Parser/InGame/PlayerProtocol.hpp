#ifndef PLAYERPROTOCOL_H
#define PLAYERPROTOCOL_H

#include "../../Core/Contexts/NetworkTask.hpp"
#include "../../Core/Contexts/Contexts.hpp"
class PlayerProtocol
{

public:
    PlayerProtocol(){}
    ~PlayerProtocol(){}

    PacketResult Test(NetworkTask& element, Context& context);    
};

#endif