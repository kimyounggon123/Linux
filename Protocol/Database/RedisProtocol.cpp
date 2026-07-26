#include "RedisProtocol.hpp"

PacketResult RedisProtocol::Test(NetElement& element, DBContext& context)
{
    return PacketResult::Success;
}