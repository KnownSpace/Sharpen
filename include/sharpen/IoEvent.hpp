#pragma once
#ifndef _SHARPEN_IOEVENT_HPP
#define _SHARPEN_IOEVENT_HPP

#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    
    class IChannel;

    class IoEvent
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
           
        EventType type_;
        sharpen::IChannel *channel_;
        void *data_;
    public:
    
        IoEvent(EventType type,sharpen::IChannel *channel,void *data)
            :type_(type)
            ,channel_(channel)
            ,data_(data)
        {}
        
        IoEvent(const Self &other)
            :type_(other.type_)
            ,channel_(other.channel_)
            ,data_(other.data_)
        {}
        
        IoEvent(Self &&other) noexcept
            :type_(other.type_)
            ,channel_(other.channel_)
            ,data_(other.data_)
        {
            other.type_ = EventTypeEnum::None;
            other.channel_ = nullptr;
            other.data_ = nullptr;
        }
        
        Self &operator=(const Self &other)
        {
            this->type_ = other.type_;
            this->channel_ = other.channel_;
            this->data_ = other.data_;
            return *this;
        }
        
        Self &operator=(Self &&other) noexcept
        {
            this->type_ = other.type_;
            this->channel_ = other.channel_;
            this->data_ = other.data_;
            other.type_ = EventTypeEnum::None;
            other.channel_ = nullptr;
            other.data_ = nullptr;
            return *this;
        }
        
        ~IoEvent() noexcept = default;
        
        bool IsReadEvent() const noexcept
        {
            return this->type_ & EventTypeEnum::Read;
        }
        
        bool IsWriteEvent() const noexcept
        {
            return this->type_ & EventTypeEnum::Write;
        }
        
        bool IsCloseEvent() const noexcept
        {
            return this->type_ & EventTypeEnum::Close;
        }
        
        bool IsErrorEvent() const noexcept
        {
            return this->type_ & EventType::Error;
        }
        
        sharpen::IChannel *GetChannel() const noexcept
        {
            return this->channel_;
        }
        
        void *GetData() const noexcept
        {
            return this->data_;
        }
    };
}

#endif
