#ifndef MANAGERTHREAD_H
#define MANAGERTHREAD_H


#include <vector>
#include "IManager.hpp"
#include "../../Utils/Thread/ThreadPool.hpp"
class ManagerThread : public BasicThreadPoolElement
{
    std::vector<IManager*> managers;

    void Work() override;
public:
    ManagerThread() : BasicThreadPoolElement(0) {}
    ~ManagerThread()
    {
        managers.clear();
        //std::cout << "Manager Thread" << std::endl;
    }
    bool AddManager(IManager* manager)
    {
        if (manager == nullptr) return false;
        managers.push_back(manager);
        return true;
    }
    void Clear() {managers.clear();}
};

#endif