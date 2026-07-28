#include "RedisProtocol.hpp"

PacketResult RedisProtocol::Test(NetElement& element, DBContext& context)
{
    std::cout << "hello in redis" << std::endl;
    return PacketResult::Success;
}