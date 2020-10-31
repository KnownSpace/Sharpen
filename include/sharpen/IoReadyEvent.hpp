#pragma once
#ifndef _SHARPEN_IOREADYEVENT
#define _SHARPEN_IOREADYEVENT

#include "IoEvent.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "FileTypeDef.hpp"

namespace sharpen
{
    class IoReadyEvent:public sharpen::Noncopyable,public sharpen::Nonmovable,public sharpen::IoEvent
    {
    
    //event type
    public:
        struct EventType
        {
            enum 
            {
                Read = 1,
                Write = 2,
                Close = 4,
                Error = 8
            };
        };
    
    private:
        using Self = sharpen::IoReadyEvent;
        using MyType = typename Self::EventType;
        
        sharpen::FileHandle handle_;
        MyType type_;
    public:
        IoReadyEvent(sharpen::FileHandle handle,MyType type);
        
        ~IoReadyEvent() = default;
        
        virtual void Handle(ISelector &selector) override;
    };
}

#endif
