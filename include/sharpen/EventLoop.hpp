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
        
        sharpen::ISelector *selector_;
        TaskVector tasks_;
        bool exectingTask_;
        Lock lock_;

        //one loop per thread
        static thread_local EventLoop *LocalLoop;
    public:
        EventLoop(sharpen::ISelector *selector);
        
        ~EventLoop() noexcept;
              
        void Bind(sharpen::IChannel *channel);
        
        void Unbind(sharpen::IChannel *channel) noexcept;
        
        void EnableWriteListen(sharpen::IChannel *channel);
        
        void DisableWriteListen(sharpen::IChannel *channel);
        
        void QueueInLoop(Task task);
        
        void Run();
        
        void Stop();

        static sharpen::EventLoop *GetLocalLoop() noexcept;
    };
}

#endif
