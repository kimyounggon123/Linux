#ifndef PACKETPROCESS_H
#define PACKETPROCESS_H

#include <unordered_map>
#include <functional>

#include <iostream>
#include "../../Public/PacketProcessInterface.hpp"



struct BasicContext
{
    Router& router;
    SessionManager& sessionManager;
    BasicContext(): router(Router::GetInstance()), sessionManager(SessionManager::GetInstance())
    {}

    void Delete()
    {
        router.DeleteInstance();
        sessionManager.DeleteInstance();
    }
};

class PacketProcessDispatcher : public IPacketProcessDispatcher<BasicContext>
{
public:
    using ProcessHandler = PacketResult(PacketProcessDispatcher::*)(NetElement&, BasicContext&);

private:
    static std::array<ProcessHandler, ChangeToUINT(PacketType::LastDummy)> handlers; 
    bool FindSession(NetElement& element);
    
    PacketResult HelloNewSession(NetElement& element, BasicContext& resource);
public:
    PacketProcessDispatcher(): IPacketProcessDispatcher() {}
    ~PacketProcessDispatcher() 
    {}

    bool Initialize() override;
    PacketResult Dispatch(NetElement& element, BasicContext& resource) override;
};





/*
template <typename MapKey, typename FuncElement>
class IPacketProcess
{
    std::unordered_map<MapKey, std::function<PROCESS_RESULT(FuncElement&)>> func_map;
       
protected:
    Router& router;
    SessionManager& sessionManager;
    PROCESS_RESULT NULL_PACKET_METHOD(FuncElement& param)
    {
        //std::cout << "ERROR Packet!!!" << std::endl;
        return PROCESS_RESULT::NULL_METHOD;
    }
    bool EmplaceFuncion(const MapKey key, std::function<PROCESS_RESULT(FuncElement&)> func)
    {
        auto pair = func_map.emplace(key, func);
        return pair.second;
    } 
public:
    IPacketProcess():
        router(Router::GetInstance()),
        sessionManager(SessionManager::GetInstance()) 
    {}
    virtual ~IPacketProcess() 
    {
        func_map.clear();
    }
    virtual bool Initialize() = 0;
    std::function<PROCESS_RESULT(FuncElement&)> GetFunc(const MapKey& key)
    {
        auto func = func_map.find(key);
        if (func != func_map.end()) return func->second;

        // 수상한 헤더 포착 시 에러 method 호출
        return [this](FuncElement& element){return this->NULL_PACKET_METHOD(element);};
    }
};


class PacketProcess : public IPacketProcess<PacketType, NetElement>
{
    bool FindSession(NetElement& param);

    PROCESS_RESULT TryHelloNewSession(NetElement& param);
    PROCESS_RESULT GetResultTryHelloNewSession(NetElement& param);

public:
    PacketProcess(): IPacketProcess() {}
    ~PacketProcess() {}

    bool Initialize() override;
    //std::function<SERVER_ERROR(NetElement&)> GetFunc(const PacketType& key);
};
*/

#endif