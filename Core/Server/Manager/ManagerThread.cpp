#include "ManagerThread.hpp"


void ManagerThread::Work()
{
    while(isRunning)
    {
        for (auto* manager : managers)
        {
            manager->Process();
        }
        ThreadUtil::SleepMs(1000);
    }
}