#ifndef DATABASEJOB_H
#define DATABASEJOB_H

enum class DatabaseAgentID
{
    LogIn,
    LAST_DUMMY
};


struct SignInJob
{

};
struct DatabaseJob
{
    DatabaseAgentID id;

    union
    {
        SignInJob sign;
    };
    
};


#endif