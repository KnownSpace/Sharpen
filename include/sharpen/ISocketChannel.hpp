#pragma once
#ifndef _SHARPEN_ISOCKETCHANNEL_HPP
#define _SHARPEN_ISOCKETCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "IAsyncReadable.hpp"
#include "IAsyncWritable.hpp"
#include "IFileChannel.hpp"

namespace sharpen
{

    class ISocketChannel;

    using SocketChannelPtr = std::shared_ptr<sharpen::ISocketChannel>;

    class ISocketChannel:public sharpen::IChannel,public sharpen::IAsyncWritable,public sharpen::IAsyncReadable
    {
    private:
        using Self = sharpen::ISocketChannel;
    public:
        
        ISocketChannel() = default;
        
        virtual ~ISocketChannel() noexcept = default;
        
        ISocketChannel(const Self &) = default;
        
        ISocketChannel(Self &&) noexcept = default;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future) = 0;
        
        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future) = 0;

        void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset);

        void SendFileAsync(sharpen::FileChannelPtr file);

        virtual void AcceptAsync(sharpen::Future<sharpen::SocketChannelPtr> &future) = 0;

        sharpen::SocketChannelPtr AcceptAsync();

        virtual void ConnectAsync(sharpen::Future<void> &future) = 0;

        void ConnectAsync();
    };
}

#endif
