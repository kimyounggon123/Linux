#ifndef REDISPROTOCOL_H
#define REDISPROTOCOL_H


#include "../../Core/Contexts/NetElement.hpp"
#include "../../Core/Contexts/Contexts.hpp"

class RedisProtocol
{
public:
    RedisProtocol(){}
    ~RedisProtocol(){}


    PacketResult Test(NetElement& element, DBContext& context);
};

#endif