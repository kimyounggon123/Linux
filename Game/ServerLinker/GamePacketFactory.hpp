#ifndef GAMEPACKETFACTORY_H
#define GAMEPACKETFACTORY_H

#include "../../Core/Packet/IPacketFactory.hpp"
#include "../Dispatcher/GameTask.hpp"
class GamePacketFactory : public IPacketFactory<GamePacketFactory, GameTask, ChangeToUINT(GameTaskType::LAST_DUMMY)>
{
    bool Test(Packet& pk, GameTask& task);
public:
    GamePacketFactory(PacketPool& pkPool): IPacketFactory(pkPool){}
    ~GamePacketFactory() {}
    bool Initialize() override;
};

#endif