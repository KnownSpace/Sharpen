#pragma once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "FileTypeDef.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace sharpen
{
    class EventLoop;

    class IoEvent;

    class IEventLoopGroup;

    void CloseFileHandle(sharpen::FileHandle handle) noexcept;

    class IChannel : public std::enable_shared_from_this<sharpen::IChannel>
    {
    private:
        using Self = sharpen::IChannel;
        using Closer = std::function<void(sharpen::FileHandle)>;

    protected:
        sharpen::EventLoop *loop_;

        sharpen::FileHandle handle_;

        Closer closer_;

    public:
        IChannel() noexcept = default;

        IChannel(const Self &) = default;

        IChannel(Self &&) noexcept = default;

        virtual ~IChannel() noexcept;

        virtual void OnEvent(sharpen::IoEvent *event) = 0;

        virtual void Register(sharpen::EventLoop &loop);

        void Register(sharpen::IEventLoopGroup &loopGroup);

        // close channel
        void Close() noexcept;

        sharpen::FileHandle GetHandle() const noexcept
        {
            return this->handle_;
        }

        sharpen::EventLoop *GetLoop() noexcept
        {
            return this->loop_;
        }

        bool IsRegistered() const noexcept
        {
            return this->loop_ != nullptr;
        }

        bool IsClosed() const noexcept;
    };

    using ChannelPtr = std::shared_ptr<sharpen::IChannel>;
}   // namespace sharpen

#endif
