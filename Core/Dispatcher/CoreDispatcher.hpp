#ifndef DISPATCHERBASE_H
#define DISPATCHERBASE_H

#include "NetworkTask.hpp"
#include "../CoreServices.hpp"
#include "../Utils/Log.hpp"

class CoreDispatcher
{
public:
    using Handler = PacketResult(CoreDispatcher::*)(NetworkTask&);

protected:
    static constexpr uint32_t MaxSize =  ChangeToUINT(PacketType::LAST_DUMMY);
    
    CoreServices& core;
    ComponentConnections connections;

    std::array<Handler, MaxSize> handlers;
    bool isInitialized;
    
    PacketResult Test(NetworkTask& task);
    PacketResult NULL_METHOD(NetworkTask& task) 
    {
        task.pk->PrintInformation();
         return PacketResult::CALL_NULL_METHOD;
    }
public:
    CoreDispatcher(CoreServices& core, const ComponentConnections& connections):
        core(core), connections(connections), isInitialized(false) 
    {
        handlers.fill(&CoreDispatcher::NULL_METHOD);
    }

    ~CoreDispatcher() {}
    bool Initialize();

    PacketResult Dispatch(uint32_t taskID, NetworkTask& task) 
    {
        if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(task);
        return (this->*handlers[taskID])(task);
    }
};


// template <typename Derived, typename T, typename C, typename R, size_t DispatcherSize>
// class DispatcherBase
// {
// public:
//     using Handler = R(Derived::*)(T&, C&);
// protected:
//     CoreServices& services;
//     std::array<Handler, DispatcherSize> handlers{};
//     bool isInitialized = false;

//     virtual R NULL_METHOD(T& task, C& context) = 0;
// public:
//     DispatcherBase(CoreServices& services): services(services)
//     {}
//     virtual ~DispatcherBase() = default;

//     R Dispatch(uint32_t taskID, T& task, C& context)
//     {
//         if (taskID >= DispatcherSize || handlers[taskID] == nullptr) return NULL_METHOD(task, context);
//         return (static_cast<Derived*>(this)->*handlers[taskID])(task, context);
//     }
//     virtual bool Initialize() = 0;
// };
#endif