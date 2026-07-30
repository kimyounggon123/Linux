#ifndef REDISPROTOCOL_H
#define REDISPROTOCOL_H


#include "../../Core/Contexts/NetworkTask.hpp"
#include "../../Core/Contexts/Contexts.hpp"

class RedisProtocol
{
public:
    RedisProtocol(){}
    ~RedisProtocol(){}


    PacketResult Test(NetworkTask& element, DBContext& context);
};

#endif