#pragma once
#ifndef _SHARPEN_WINNETSTREAMCHANNEL_HPP
#define _SHARPEN_WINNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_WINSOCKET

#include "INetStreamChannel.hpp"
#include "IocpOverlappedStruct.hpp"

namespace sharpen
{
    class WinNetStreamChannel:public sharpen::INetStreamChannel,public sharpen::Noncopyable
    {
    private:
        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event,sharpen::Uint64 offset);
    public:
        explicit WinNetStreamChannel(sharpen::FileHandle handle);

        ~WinNetStreamChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future) override;
        
        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future) override;

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) override;

        virtual void ConnectAsync(sharpen::Future<void> &future) override;
    };
};

#endif
#endif
