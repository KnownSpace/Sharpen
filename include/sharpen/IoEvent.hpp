#pragma once
#ifndef _SHARPEN_IOEVENT_HPP
#define _SHARPEN_IOEVENT_HPP

namespace sharpen
{
    class ISelector;

    class IoEvent
    {
    private:
        using Self = sharpen::IoEvent;
    public:
    
        IoEvent() = default;
        
        IoEvent(const Self &) = default;
        
        IoEvent(Self &&) noexcept = default;
        
        virtual ~IoEvent() = default;
        
        virtual void Handle(sharpen::ISelector &selector) = 0;
    };
}

#endif
