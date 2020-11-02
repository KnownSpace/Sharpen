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
              
        virtual void Bind(sharpen::IChannel *channel) = 0;
        
        virtual void Unbind(sharpen::IChannel *channel) noexcept = 0;
        
        virtual void EnableWriteListen(sharpen::IChannel *channel) = 0;
        
        virtual void DisableWriteListen(sharpen::IChannel *channel) = 0;
        
        virtual void QueueInLoop(Task task) = 0;
        
        virtual void Run() = 0;
        
        virtual void Stop() = 0;
    };
}

#endif
