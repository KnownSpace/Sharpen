#pragma once
#ifndef _SHARPEN_ISELECTOR_HPP
#define _SHARPEN_ISELECTOR_HPP

#include <vector>
#include <memory>

#include "IoEvent.hpp"
#include "FileTypeDef.hpp"

namespace sharpen
{
    class IChannel;

    class ISelector
    {
    private:
        using Self = sharpen::ISelector;
        using EventPtr = std::unique_ptr<sharpen::IoEvent>;
        using EventVector = std::vector<EventPtr>;
    public:
    
        ISelector() = default;
        
        ISelector(const Self &) = default;
        
        ISelector(Self &&) noexcept = default;
        
        virtual ~ISelector() noexcept = default;
        
        //select events and save event to events
        virtual void Select(EventVector &events) = 0;
        
        //queue task to selector
        virtual void QueueInLoop(std::function<void()> fn) = 0;
        
        //register file handle
        virtual void Resister(sharpen::IChannel *channel) = 0;
        
        //unregister file handle
        virtual void Unregister(sharpen::IChannel *channel) = 0;
        
        //enable writing listen
        virtual void EnableWriteListen(sharpen::IChannel *channel) = 0;
        
        //disable writing listen
        virtual void DisableWritelisten(sharpen::IChannel *channel) = 0;
        
    }
}

#endif
