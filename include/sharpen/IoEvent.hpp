#pragma once
#ifndef _SHARPEN_IOEVENT_HPP
#define _SHARPEN_IOEVENT_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SystemError.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen
{

    class IoEvent
    {
    public:
        // event type
        struct EventTypeEnum
        {
            enum
            {
                // empty event
                None = 0,
                // read event
                Read = 1,
                // write event
                Write = 2,
                // close by peer
                Close = 4,
                // error
                Error = 8,

                // iocp only
                // io completed
                Completed = 16,
                // io request
                Request = 32,

                // accept handle
                Accept = 64,
                // connect
                Connect = 128,
                // send file
                Sendfile = 256,
                // poll
                Poll = 512,

                // io_uring only
                // flush
                Flush = 1024
            };
        };

        using EventType = std::uint32_t;

    private:
        using Self = sharpen::IoEvent;
        using WeakChannel = std::weak_ptr<sharpen::IChannel>;

        EventType type_;
        WeakChannel channel_;
        void *data_;
        sharpen::ErrorCode errorCode_;

    public:
        IoEvent()
            : type_(EventTypeEnum::None)
            , channel_()
            , data_(nullptr)
            , errorCode_(0)
        {
        }

        IoEvent(EventType type, sharpen::ChannelPtr channel, void *data, sharpen::ErrorCode error)
            : type_(type)
            , channel_(channel)
            , data_(data)
            , errorCode_(error)
        {
        }

        IoEvent(const Self &other)
            : type_(other.type_)
            , channel_(other.channel_)
            , data_(other.data_)
            , errorCode_(other.errorCode_)
        {
        }

        IoEvent(Self &&other) noexcept
            : type_(other.type_)
            , channel_(other.channel_)
            , data_(other.data_)
            , errorCode_(other.errorCode_)
        {
            other.type_ = EventTypeEnum::None;
            other.channel_.reset();
            other.data_ = nullptr;
            other.errorCode_ = 0;
        }

        Self &operator=(const Self &other)
        {
            this->type_ = other.type_;
            this->channel_ = other.channel_;
            this->data_ = other.data_;
            this->errorCode_ = other.errorCode_;
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            this->type_ = other.type_;
            this->channel_ = other.channel_;
            this->data_ = other.data_;
            this->errorCode_ = other.errorCode_;
            other.type_ = EventTypeEnum::None;
            other.channel_.reset();
            other.data_ = nullptr;
            other.errorCode_ = 0;
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
            return this->type_ & EventTypeEnum::Error;
        }

        bool IsCompletedEvent() const noexcept
        {
            return this->type_ & EventTypeEnum::Completed;
        }

        bool IsRequestEvent() const noexcept
        {
            return this->type_ & EventTypeEnum::Request;
        }

        sharpen::ChannelPtr GetChannel() const noexcept
        {
            return this->channel_.lock();
        }

        void *GetData() const noexcept
        {
            return this->data_;
        }

        void SetData(void *data)
        {
            this->data_ = data;
        }

        sharpen::ErrorCode GetErrorCode() const noexcept
        {
            return this->errorCode_;
        }

        void SetErrorCode(sharpen::ErrorCode error)
        {
            this->errorCode_ = error;
        }

        void SetChannel(const sharpen::ChannelPtr &channel)
        {
            this->channel_ = channel;
        }

        void SetEvent(EventType ev)
        {
            this->type_ = ev;
        }

        EventType GetEventType() const noexcept
        {
            return this->type_;
        }

        void AddEvent(EventType ev)
        {
            this->type_ |= ev;
        }

        void RemoveEvent(EventType ev)
        {
            this->type_ ^= ev;
        }
    };
}   // namespace sharpen

#endif
