#pragma once
#ifndef _SHARPEN_WINNETSTREAMCHANNEL_HPP
#define _SHARPEN_WINNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_WINSOCKET

#include "INetStreamChannel.hpp"
#include "WSAOverlappedStruct.hpp"

namespace sharpen
{
    class WinNetStreamChannel:public sharpen::INetStreamChannel,public sharpen::Noncopyable
    {
    private:
        using Mybase = sharpen::INetStreamChannel;

        static void Closer(sharpen::FileHandle handle) noexcept;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::WSAOverlappedStruct &olStruct);

        void HandleReadAndWrite(WSAOverlappedStruct &olStruct);

        void HandleAccept(WSAOverlappedStruct &olStruct);

        void HandleSendFile(WSAOverlappedStruct &olStruct);

        void HandleConnect(WSAOverlappedStruct &olStruct);

        void HandlePoll(WSAOverlappedStruct &olStruct);

        void RequestRead(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestWrite(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestSendFile(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> *future);

        void RequestConnect(const sharpen::IEndPoint *endpoint,sharpen::Future<void> *future);

        void RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future);

        void RequestPollRead(sharpen::Future<void> *future);

        void RequestPollWrite(sharpen::Future<void> *future);

        void RequestCancel() noexcept;

        int af_;
    public:
        WinNetStreamChannel(sharpen::FileHandle handle,int af);

        ~WinNetStreamChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future) override;
        
        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future) override;

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) override;

        virtual void ConnectAsync(const sharpen::IEndPoint &endpoint,sharpen::Future<void> &future) override;
    
        virtual void PollReadAsync(sharpen::Future<void> &future) override;

        virtual void PollWriteAsync(sharpen::Future<void> &future) override;

        virtual void Cancel() noexcept override;
    };
};

#endif
#endif
