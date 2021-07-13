#pragma once
#ifndef _SHARPEN_POSIXNETSTREAMCHANNEL_HPP
#define _SHARPEN_POSIXNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXSOCKET

#include "INetStreamChannel.hpp"
#include "PosixIoReader.hpp"
#include "PosixIoWriter.hpp"

#include <list>
#include <sys/uio.h>
#include <atomic>

namespace sharpen
{

    class PosixNetStreamChannel:public sharpen::INetStreamChannel,public sharpen::Noncopyable
    {
    private:
        using Mybase = sharpen::INetStreamChannel;
        using Lock = sharpen::SpinLock;
        using AcceptCallback = std::function<void(sharpen::FileHandle)>;
        using ConnectCallback = std::function<void()>;
        using StatusBit = std::atomic_bool;
        using Callback = std::function<void(ssize_t)>;

        enum class IoStatus
        {
            Io,
            Accept,
            Connect
        };

        //status
        bool writeable_;
        bool readable_;
        IoStatus status_;
        //operator
        sharpen::PosixIoReader reader_;
        sharpen::PosixIoWriter writer_;
        //callback
        AcceptCallback acceptCb_;
        ConnectCallback connectCb_;

        sharpen::FileHandle DoAccept();

        void DoRead();

        void DoWrite();

        void HandleRead();

        void HandleAccept();

        void HandleWrite();

        void HandleClose() noexcept;

        bool HandleConnect();

        void TryRead(char *buf,sharpen::Size bufSize,Callback cb);

        void TryWrite(const char *buf,sharpen::Size bufSize,Callback cb);

        void TryAccept(AcceptCallback cb);

        void TryConnect(const sharpen::IEndPoint &endPoint,ConnectCallback cb);

        void RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestSendFile(sharpen::FileHandle handle,sharpen::Uint64 offset,sharpen::Size size,sharpen::Future<void> *future);

        void RequestConnect(const sharpen::IEndPoint &endPoint,sharpen::Future<void> *future);

        void RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future);

        static void CompleteConnectCallback(sharpen::Future<void> *future) noexcept;

        static void CompleteIoCallback(sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept;

        static void CompleteSendFileCallback(sharpen::Future<void> *future,void *mem,sharpen::Size memLen,ssize_t) noexcept;

        static void CompleteAcceptCallback(sharpen::Future<sharpen::NetStreamChannelPtr> *future,sharpen::FileHandle accept) noexcept;

    public:

        explicit PosixNetStreamChannel(sharpen::FileHandle handle);

        ~PosixNetStreamChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future) override;
        
        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future) override;

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) override;

        virtual void ConnectAsync(const sharpen::IEndPoint &endpoint,sharpen::Future<void> &future) override;

        virtual void Listen(sharpen::Uint16 queueLength) override;
    };
};

#endif
#endif
