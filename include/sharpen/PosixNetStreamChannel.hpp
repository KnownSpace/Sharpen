#pragma once
#ifndef _SHARPEN_POSIXNETSTREAMCHANNEL_HPP
#define _SHARPEN_POSIXNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXSOCKET

#include "INetStreamChannel.hpp"

#include <list>
#include <sys/uio.h>
#include <atomic>

namespace sharpen
{

    class PosixNetStreamChannel:public sharpen::INetStreamChannel,public sharpen::Noncopyable
    {
    private:
        using Mybase = sharpen::INetStreamChannel;
        using Buffers = std::list<iovec>;
        using Lock = sharpen::SpinLock;
        using Callback = std::function<void(sharpen::Size)>;
        using Callbacks = std::list<Callback>;
        using AcceptCallback = std::function<void(sharpen::FileHandle)>;
        using ConnectCallback = std::function<void()>;
        using StatusBit = std::atomic_bool;

        enum class IoStatus
        {
            Io,
            Accept,
            Connect
        };

        //locks
        Lock writeLock_;
        Lock readLock_;
        //status
        bool readable_;
        bool writeable_;
        IoStatus status_;
        bool error_;
        bool closed_;
        sharpen::Uint16 acceptCount_;
        //buffers
        Buffers readBuffers_;
        Buffers pendingReadBuffers_;
        Buffers writeBuffers_;
        Buffers pendingWriteBuffers_;
        //callback
        AcceptCallback acceptCb_;
        ConnectCallback connectCb_;
        Callbacks readCbs_;
        Callbacks pendingReadCbs_;
        Callbacks writeCbs_;
        Callbacks pendingWriteCbs_;
        

        static sharpen::Size SetIovecs(iovec *vecs,Buffers &buf);

        static sharpen::Size ConvertBytesToBufferNumber(sharpen::Size bytes,sharpen::Size &lastOffset,Buffers &buf);

        static void FillBuffer(Buffers &buf,Buffers &pending);

        static void FillCallback(Callbacks &cbs,Callbacks &pending);

        sharpen::Size DoWrite(sharpen::Size &lastSize);

        sharpen::Size DoRead(sharpen::Size &lastSize);

        sharpen::FileHandle DoAccept();

        void HandleRead();

        void HandleAccept();

        void HandleWrite();

        void HandleClose() noexcept;

        bool HandleConnect();

        void RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        void RequestSendFile(sharpen::FileHandle handle,sharpen::Uint64 offset,sharpen::Size size,sharpen::Future<void> *future);

        void RequestConnect(const sharpen::IEndPoint &endPoint,sharpen::Future<void> *future);

        void RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future);

        static void CompleteConnectCallback(sharpen::Future<void> *future) noexcept;

        static void CompleteIoCallback(sharpen::Future<sharpen::Size> *future,sharpen::Size size) noexcept;

        static void CompleteSendFileCallback(sharpen::Future<void> *future,void *mem,sharpen::Size memLen,sharpen::Size) noexcept;

        static void CompleteAcceptCallback(sharpen::Future<sharpen::NetStreamChannelPtr> *future,sharpen::FileHandle accept) noexcept;

        static bool ErrorBlocking();
    protected:

    public:

        explicit PosixNetStreamChannel(sharpen::FileHandle handle);

        ~PosixNetStreamChannel() noexcept;

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
