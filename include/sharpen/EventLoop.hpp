#pragma once
#ifndef _SHARPEN_IEVENTLOOP_HPP
#define _SHARPEN_IEVENTLOOP_HPP

#include <functional>
#include <vector>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"

namespace sharpen
{
    
    class IChannel;
    
    class ISelector;
    
    class EventLoop:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Task = std::function<void()>;
        using Lock = sharpen::SpinLock;
        using TaskVector = std::vector<Task>;
        using TaskVectorPrr = std::shared_ptr<TaskVector>;
        using LockPtr = std::shared_ptr<Lock>;
        using SelectorPtr = std::shared_ptr<shared::ISelector>;
        
        SelectorPtr selector_;
        TaskVectorPtr tasks_;
        bool exectingTask_;
        LockPtr lock_;
        bool running_;

        //one loop per thread
        static thread_local EventLoop *LocalLoop;

        //execute pending tasks
        void ExecuteTask();
    public:
        //create event loop with a selector and an uniqued task list
        explicit EventLoop(SelectorPtr selector);
        
        //create event loop with a selector and an shared task list
        EventLoop(SelectorPtr selector,TaskVectorPtr tasks,LockPtr lock);
        
        ~EventLoop() noexcept;
           
        //bind a channel to event loop
        //the channel must be supported by selector
        void Bind(sharpen::IChannel *channel);
        
        //unbind a channel
        void Unbind(sharpen::IChannel *channel) noexcept;
        
        //start listen write event on channel
        //it will be ignored if IOCP is available
        void EnableWriteListen(sharpen::IChannel *channel);
        
        //stop listen write event on channel
        //it will be ignored if IOCP is available
        void DisableWriteListen(sharpen::IChannel *channel);
        
        //queue a task to event loop
        //the task will be executed in next loop
        void QueueInLoop(Task task);
        
        //run event loop in this thread
        void Run();
        
        //stop event loop
        void Stop();

        //get thread local event loop
        static sharpen::EventLoop *GetLocalLoop() noexcept;
    };
}

#endif
