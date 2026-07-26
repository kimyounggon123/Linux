#ifndef PLAYERPROTOCOL_H
#define PLAYERPROTOCOL_H

#include "../../Core/Contexts/NetElement.hpp"
#include "../../Core/Contexts/Contexts.hpp"
class PlayerProtocol
{

public:
    PlayerProtocol(){}
    ~PlayerProtocol(){}

    PacketResult Test(NetElement& element, Context& context);    
};

#endif