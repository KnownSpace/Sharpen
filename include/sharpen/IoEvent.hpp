#pragma once
#ifndef _SHARPEN_IOEVENT_HPP
#define _SHARPEN_IOEVENT_HPP

#include "TypeDef.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class ISelector;

    class IoEvent:public sharpen::Noncopyable
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
    public:
    
        explicit IoEvent(EventType type)
            :type_(type)
        {}
        
        IoEvent(Self &&other) noexcept
            :type_(other.type_)
        {
            other.type_ = EventTypeEnum::None;
        }
        
        Self &operator=(Self &&other) noexcept
        {
            this->type_ = other.type_;
            other.type_ = EventTypeEnum::None;
            return *this;
        }
        
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
    };
}

#endif
