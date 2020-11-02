#pragma once
#ifndef _SHARPEN_IEVENTLOOP_HPP
#define _SHARPEN_IEVENTLOOP_HPP

#include <functional>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    
    class IChannel;
    
    class IEventLoop:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Task = std::function<void()>;
    public:
        IEventLoop() = default;
        
        virtual ~IEventLoop() noexcept = default;
              
        virtual void Register(IChannel *channel) = 0;
        
        virtual void Unregister(IChannel *channel) noexcept = 0;
        
        virtual void EnableWriteListen(IChannel *channel) = 0;
        
        virtual void DisableWriteListen(IChannel *channel) = 0;
        
        virtual void QueueInLoop(Task task) = 0;
        
        virtual void Run() = 0;
        
        virtual void Stop() = 0;
    };
}

#endif
