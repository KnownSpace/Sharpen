#pragma once
#ifndef _SHARPEN_IOEVENT_HPP
#define _SHARPEN_IOEVENT_HPP

#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class ISelector;
    
    class IChannel;

    class IoEvent:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    public:
        //event type   
        struct EventTypeEnum
        {
            enum 
            {
                //empty event
                None = 0
                //read event
                Read = 1,
                //write event
                Write = 2,
                //close by peer
                Close = 4,
                //error
                Error = 8
            };
        };
        
        using EventType = sharpen::Uint32;
    private:
        using Self = sharpen::IoEvent;
        
    protected:    
        EventType type_;
        sharpen::IChannel *channel_;
    public:
    
        IoEvent(EventType type,sharpen::IChannel *channel)
            :type_(type)
            ,channel_(channel)
        {}
        
        virtual ~IoEvent() = default;
        
        //do some works if necessary
        //do nothing by default
        virtual void Handle(sharpen::ISelector &selector)
        {}
        
        bool IsReadEvent() const
        {
            return this->type_ & EventTypeEnum::Read;
        }
        
        bool IsWriteEvent() const
        {
            return this->type_ & EventTypeEnum::Write;
        }
        
        bool IsCloseEvent() const
        {
            return this->type_ & EventTypeEnum::Close;
        }
        
        bool IsErrorEvent() const
        {
            return this->type_ & EventType::Error;
        }
        
        sharpen::IChannel *GetChannel() noexcept
        {
            return this->channel_;
        }
    };
}

#endif
