#pragma once
#ifndef _SHARPEN_IEVENTLOOP_HPP
#define _SHARPEN_IEVENTLOOP_HPP

#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include "ISelector.hpp"
#include "IChannel.hpp"
#include "IoEvent.hpp"
#include "Fiber.hpp"

namespace sharpen
{
    
    class EventLoop:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Task = std::function<void()>;
        using Lock = sharpen::SpinLock;
        using TaskVector = std::vector<Task>;
        using SelectorPtr = std::shared_ptr<sharpen::ISelector>;
        using EventVector = std::vector<sharpen::IoEvent*>;
        using WeakChannelPtr = std::weak_ptr<sharpen::IChannel>;
        
        SelectorPtr selector_;
        TaskVector tasks_;
        TaskVector pendingTasks_;
        bool exectingTask_;
        Lock lock_;
        bool running_;
        bool waiting_;

        //one loop per thread
        thread_local static EventLoop *localLoop_;

        thread_local static sharpen::FiberPtr localFiber_;

        //execute pending tasks
        void ExecuteTask();
    public:
        //create event loop with a selector and an uniqued task list
        explicit EventLoop(SelectorPtr selector);
        
        ~EventLoop() noexcept;
           
        //bind a channel to event loop
        //the channel must be supported by selector
        void Bind(WeakChannelPtr channel);
        
        sharpen::ISelector &GetSelector() const noexcept
        {
            return *this->selector_;
        }
        
        //queue a task to event loop
        //the task will be executed in next loop
        //if this thread own the loop
        //task will be executed right now
        void RunInLoop(Task task);

        //queue a task to event loop
        //the task will be executed in next loop
        void RunInLoopSoon(Task task);
        
        //run event loop in this thread
        void Run();
        
        //stop event loop
        void Stop() noexcept;

        //get thread local event loop
        static sharpen::EventLoop *GetLocalLoop() noexcept;

        static bool IsInLoop() noexcept;

        static sharpen::FiberPtr GetLocalFiber() noexcept;

        bool IsWaiting() const noexcept;
    };
}

#endif