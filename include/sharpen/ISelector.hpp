#pragma once
#ifndef _SHARPEN_ISELECTOR_HPP
#define _SHARPEN_ISELECTOR_HPP

#include <vector>
#include <memory>

#include "IoEvent.hpp"
#include "FileTypeDef.hpp"
#include "IChannel.hpp"

namespace sharpen
{
    class ISelector
    {
    private:
        using Self = sharpen::ISelector;
        
    protected:
        using Event = sharpen::IoEvent;
        using EventVector = std::vector<Event*>;
        using WeakChannelPtr = std::weak_ptr<sharpen::IChannel>;

    public:
    
        ISelector() = default;
        
        ISelector(const Self &) = default;
        
        ISelector(Self &&) noexcept = default;
        
        virtual ~ISelector() noexcept = default;
        
        //select events and save to events
        virtual void Select(EventVector &events) = 0;
        
        //notify io thread
        virtual void Notify() = 0;
        
        //register file handle
        virtual void Resister(WeakChannelPtr channel) = 0;
    };

    using SelectorPtr = std::shared_ptr<sharpen::ISelector>;

    sharpen::SelectorPtr MakeDefaultSelector();
}

#endif
