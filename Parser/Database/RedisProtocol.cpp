#include "RedisProtocol.hpp"

PacketResult RedisProtocol::Test(NetworkTask& element, DBContext& context)
{
    std::cout << "hello in redis" << std::endl;
    return PacketResult::Success;
}