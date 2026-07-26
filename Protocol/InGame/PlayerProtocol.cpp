#include "PlayerProtocol.hpp"
PacketResult PlayerProtocol::Test(NetElement& element, Context& context)
{
    std::cout << "hello in player" << std::endl;
    element.GoToHere(ElementStage::Database);
    return PacketResult::Success;
}