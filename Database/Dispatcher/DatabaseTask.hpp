#ifndef DATABASETASK_H
#define DATABASETASK_H

#include "../../Core/Packet/Packet.hpp"
#include "../../Core/Session/BasicSession.hpp"
struct SignInTask
{
    int id;
    int pw;
};



enum class DatabaseTaskType
{
    Test,
    Login,
    LAST_DUMMY
};
enum class DatabaseTaskResult
{   
    Success,

	SERVER_IS_CLOSED, // 실패의 경우 실패 요인을 code로 작성.

	DELETED_PACKET,
	OTHER_ERROR,

	PARAMETER_ERROR,
	CALL_NULL_METHOD,
	RESOURCE_FAMINE,
	INVALID_CLIENT,
	BANNED,

	LAST_DUMMY // Put this the last.
};


struct DatabaseTask
{
    bool ignoreThis;
    DatabaseTaskType type;
    DatabaseTaskResult result;

    uint32_t sessionID;
    union
    {
        SignInTask sign;
    };
    
    DatabaseTask(uint32_t sessionID): ignoreThis(false),
        type(DatabaseTaskType::LAST_DUMMY), result(DatabaseTaskResult::LAST_DUMMY),
        sessionID(sessionID)
    {}
    DatabaseTask(DatabaseTaskType type, uint32_t sessionID): ignoreThis(false),
        type(type), result(DatabaseTaskResult::LAST_DUMMY),
        sessionID(sessionID)
    {
    }

    ~DatabaseTask()
    {}
    
};


#endif