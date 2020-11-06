#pragma once
#ifndef _SHARPEN_IEVENTLOOP_HPP
#define _SHARPEN_IEVENTLOOP_HPP

#include <functional>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    
    class IChannel;
    
    class ISelector;
    
    class EventLoop:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Task = std::function<void()>;
        
        sharpen::ISelector *selector_;
    public:
        EventLoop() = default;
        
        ~EventLoop() noexcept = default;
              
        void Bind(sharpen::IChannel *channel);
        
        void Unbind(sharpen::IChannel *channel) noexcept;
        
        void EnableWriteListen(sharpen::IChannel *channel);
        
        void DisableWriteListen(sharpen::IChannel *channel);
        
        void QueueInLoop(Task task);
        
        void Run();
        
        void Stop();
    };
    
    extern thread_local EventLoop *LocalEventLoop;
}

#endif
