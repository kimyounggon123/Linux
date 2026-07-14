#ifndef PACKETPROCESS_H
#define PACKETPROCESS_H

#include <unordered_map>
#include <functional>

#include <iostream>
#include "Session.hpp"
#include "Router.hpp"


enum class PROCESS_RESULT
{
    // success return value
    SUCCESS,                    // 보통의 성공할 경우.
    GOTO_DB_PROCESS_THREAD,     // DB 작업이 필요할 경우에 DB process pool에 신청.
    SUCCESS_DB_PROCESS,         // DB 작업이 성공할 경우
    FAIL_DB_PROCESS,            // DB 작업이 실패할 경우

    // error return value
    NULL_METHOD,
    PARAMERTER_ERROR,
    CANNOT_FOUND_CLIENT,

    RESOURCE_FAMINE,

    POLLUTED_DATA,
    FAIL_INSERTING_DATA,

    UNKNOWN
};

// Mapkey: 원하는 함수를 가져오기 위한 키. 여기선 PacketType을 key로 삼음
// FuncElement: Method가 작업을 처리하기 위해 받는 param. 여기선 NetElement{session, packet}를 받음
template <typename MapKey, typename FuncElement>
class IPacketProcess
{
protected:
    Router& router;
    SessionManager& sessionManager;

    PROCESS_RESULT NULL_PACKET_METHOD(FuncElement& param)
    {
        return PROCESS_RESULT::NULL_METHOD;
    }
public:
    //using PacketHandler = PROCESS_RESULT(IPacketProcess<MapKey, FuncElement>::(*))(FuncElement&);

    IPacketProcess():
        router(Router::GetInstance()),
        sessionManager(SessionManager::GetInstance()) 
    {}
    virtual ~IPacketProcess() 
    {
        //func_map.clear();
    }

    virtual bool Initialize() = 0;
};

class PacketProcess : public IPacketProcess<PacketType, NetElement>
{
public:
    using ProcessHandler = PROCESS_RESULT(PacketProcess::*)(NetElement&);

private:
    // 보통 기능별로 따로 Service officer object를 따로 만듦.
    inline static std::array<ProcessHandler, ChangeToUINT(PacketType::LastDummy)> handlers; 

    bool FindSession(NetElement& param);
   

    PROCESS_RESULT TryHelloNewSession(NetElement& param);
    PROCESS_RESULT GetResultTryHelloNewSession(NetElement& param);

public:
    PacketProcess(): IPacketProcess() {}
    ~PacketProcess() 
    {}

    bool Initialize() override;
    PROCESS_RESULT Dispatch(NetElement& element);
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