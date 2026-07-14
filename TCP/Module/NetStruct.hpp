#ifndef NETSTRUCT_H
#define NETSTRUCT_H
#include "UtilsLinker.hpp"
#include "Session.hpp"
#include <chrono>

enum class ElementStage : uint8_t
{
    GeneralProcess,
    Database,
    Send,
    LAST_DUMMY
};


struct ProcessContext
{
    RedisControl* redis = nullptr;
    void EraseAll()
    {
        redis = nullptr;
    }
};


/*

    NetElement pipeline.
      [recv]
    - [Process Enqueue] - [Process Dequeue] - [Process] - 
      (DB작업 시에만 통과하는 추가 파이프라인 - [DB Enqueue] - [DB Dequeue] - [DB process])
    - [Send Enqueue] - [Send Dequeue] 
    - [Send]
*/
struct NetElement
{
    ElementStage nextStage;
    LinuxSession* session;
    Packet* pk;
    ProcessContext context;

    std::chrono::time_point<std::chrono::high_resolution_clock> recv_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> process_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> process_end_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> db_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> db_end_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> send_time;

    NetElement():
        nextStage(ElementStage::GeneralProcess),
        session(nullptr), pk(nullptr),
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        process_end_time(std::chrono::high_resolution_clock::now()),
        db_start_time(std::chrono::high_resolution_clock::now()),
        db_end_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now()),
        context()
    {
        if (session != nullptr) session->AddRef();
    }

    NetElement(ElementStage stage, LinuxSession* session, Packet* pk):
        nextStage(stage),
        session(session), pk(pk),
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        process_end_time(std::chrono::high_resolution_clock::now()),
        db_start_time(std::chrono::high_resolution_clock::now()),
        db_end_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now()),
        context()
    {
        if (session != nullptr) session->AddRef();
    }

    NetElement(const NetElement& other):
        nextStage(other.nextStage),
        session(other.session), pk(other.pk),
        recv_time(other.recv_time),
        process_start_time(other.process_start_time),
        process_end_time(other.process_end_time),
        db_start_time(other.db_start_time),
        db_end_time(other.db_end_time),
        send_time(other.send_time),
        context(other.context)
    {
        if (session != nullptr) session->AddRef();   
    }

    ~NetElement() 
    {
        if (session != nullptr) session->Release();
        context.EraseAll();
    }

    void RecordProcessStartTime() { process_start_time = std::chrono::high_resolution_clock::now(); }
    void RecordProcessEndTime() {process_end_time = std::chrono::high_resolution_clock::now();}
    void RecordDBStartTime() { db_start_time = std::chrono::high_resolution_clock::now(); }
    void RecordDBEndTime() { db_end_time = std::chrono::high_resolution_clock::now();}
    void RecordSendTime() { send_time = std::chrono::high_resolution_clock::now(); }

    std::chrono::duration<double, std::milli> GetDurationBetweenRecvAndProcess() { return process_start_time - recv_time; }
    std::chrono::duration<double, std::milli> GetProcessDuration() { return process_end_time - process_start_time; }
    std::chrono::duration<double, std::milli> GetDBDuration() { return db_end_time - db_start_time; }
    std::chrono::duration<double, std::milli> GetDurationBetweenProcessAndSend() { return send_time - process_end_time; }

    std::chrono::duration<double, std::milli> GetPipelineDuration() { return send_time - recv_time; }


    void GoToHere(const ElementStage& next) {nextStage = next;}

    void ShowTimeStamp(bool flagOnlyWarning)
    {
        double betweenRecvAndProcessStart = GetDurationBetweenRecvAndProcess().count() > 0.0 ? GetDurationBetweenRecvAndProcess().count() : 0.0;
        double processDuration = GetProcessDuration().count() > 0.0 ? GetProcessDuration().count() : 0.0;
        double dbDuration = GetDBDuration().count() > 0.0 ? GetDBDuration().count() : 0.0;
        double betweenProcessAndSend = GetDurationBetweenProcessAndSend().count() > 0.0 ? GetDurationBetweenProcessAndSend().count() : 0.0;
        double pipelineDuration = GetPipelineDuration().count() > 0.0 ? GetPipelineDuration().count() : 0.0;

        if (!flagOnlyWarning || betweenRecvAndProcessStart > 2.0) std::cout << "\033Between recv and process: \033" << betweenRecvAndProcessStart << " (ms)" << std::endl;
        if (!flagOnlyWarning || processDuration > 1.0) std::cout << "\033Process Duration: \033" << processDuration << " (ms)" << std::endl;
        if (!flagOnlyWarning || dbDuration > 3.0) std::cout << "\033DB Duration: \033" << dbDuration << " (ms)" << std::endl;
        if (!flagOnlyWarning || betweenProcessAndSend > 2.0) std::cout << "\033Between process and send: \033" << betweenProcessAndSend << " (ms)" << std::endl;
        if (!flagOnlyWarning || pipelineDuration > 10.0) std::cout << "\033Pipeline Duration: \033" << pipelineDuration << " (ms)\n" << std::endl;
           
    }
};

/*
색상	코드
검정	30
빨강	31
초록	32
노랑	33
파랑	34
자홍	35
청록	36
흰색	37
기본색으로 복원	0

std::cout << "\033[31mError\033[0m\n";
std::cout << "\033[32mOK\033[0m\n";
std::cout << "\033[33mWarning\033[0m\n";
*/


#endif