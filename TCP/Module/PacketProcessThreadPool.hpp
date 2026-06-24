#ifndef PACKETPROCESSTHREADPOOL_H
#define PACKETPROCESSTHREADPOOL_H

#include "UtilsLinker.hpp"
#include "PacketProcess.hpp"

class PacketProcessThreadElement : public BasicThreadPoolElement
{
    Router& router;

    PacketProcess* process;
    void Work() override;
public:
    PacketProcessThreadElement(uint32_t ID, PacketProcess* process):
        BasicThreadPoolElement(ID),
        process(process),
        router(Router::GetInstance())
    {}

    ~PacketProcessThreadElement()
    {}


    bool Initialize() override;
    void Destroy() override;

};





#endif