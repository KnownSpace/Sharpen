#pragma once
#ifndef _SHARPEN_POSIXNETSTREAMCHANNEL_HPP
#define _SHARPEN_POSIXNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"   // IWYU pragma: keep

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXSOCKET

#include "INetStreamChannel.hpp"
#include "PosixIoReader.hpp"
#include "PosixIoWriter.hpp"
#include <sys/uio.h>
#include <atomic>
#include <vector>

namespace sharpen {

    class PosixNetStreamChannel
        : public sharpen::INetStreamChannel
        , public sharpen::Noncopyable {
    private:
        using Self = sharpen::PosixNetStreamChannel;
        using Mybase = sharpen::INetStreamChannel;
        using Lock = sharpen::SpinLock;
        using AcceptCallback = std::function<void(sharpen::FileHandle)>;
        using ConnectCallback = std::function<void()>;
        using StatusBit = std::atomic_bool;
        using Callback = std::function<void(ssize_t)>;
        using Callbacks = std::vector<Callback>;

        enum class IoStatus {
            Io,
            Accept,
            Connect
        };

        // status
        bool readable_;
        bool writeable_;
        IoStatus status_;
        bool peerClosed_;
        // operator
        sharpen::PosixIoReader reader_;
        sharpen::PosixIoWriter writer_;
        // callback
        AcceptCallback acceptCb_;
        ConnectCallback connectCb_;
        Callbacks pollReadCbs_;
        Callbacks pollWriteCbs_;

        sharpen::FileHandle DoAccept();

        void DoRead();

        void DoWrite();

        void DoPollRead();

        void DoPollWrite();

        void HandleRead();

        void HandleAccept();

        void HandleWrite();

        void HandleClose() noexcept;

        bool HandleConnect();

        void TryRead(char *buf, std::size_t bufSize, Callback cb);

        void TryWrite(const char *buf, std::size_t bufSize, Callback cb);

        void TryAccept(AcceptCallback cb);

        void TryConnect(const sharpen::IEndPoint &endPoint, ConnectCallback cb);

        void TryPollRead(Callback cb);

        void TryPollWrite(Callback cb);

        void RequestRead(char *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future);

        void RequestWrite(const char *buf,
                          std::size_t bufSize,
                          sharpen::Future<std::size_t> *future);

        void RequestSendFile(sharpen::FileHandle handle,
                             std::uint64_t offset,
                             std::size_t size,
                             sharpen::Future<std::size_t> *future);

        void RequestConnect(const sharpen::IEndPoint &endPoint, sharpen::Future<void> *future);

        void RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future);

        void RequestPollRead(sharpen::Future<void> *future);

        void RequestPollWrite(sharpen::Future<void> *future);

        static void CompleteConnectCallback(sharpen::EventLoop *loop,
                                            sharpen::Future<void> *future) noexcept;

        static void CompleteIoCallback(sharpen::EventLoop *loop,
                                       sharpen::Future<std::size_t> *future,
                                       ssize_t size) noexcept;

        static void CompleteSendFileCallback(sharpen::EventLoop *loop,
                                             sharpen::Future<std::size_t> *future,
                                             void *mem,
                                             std::size_t memLen,
                                             ssize_t size) noexcept;

        static void CompleteAcceptCallback(sharpen::EventLoop *loop,
                                           sharpen::Future<sharpen::NetStreamChannelPtr> *future,
                                           sharpen::FileHandle accept) noexcept;

        static void CompletePollCallback(sharpen::EventLoop *loop,
                                         sharpen::Future<void> *future,
                                         ssize_t size) noexcept;

        static bool IsAcceptBlock(sharpen::ErrorCode err) noexcept;

        void DoCancel(sharpen::ErrorCode err) noexcept;

        void DoSafeCancel(sharpen::ErrorCode err, sharpen::ChannelPtr keepalive) noexcept;

        void SafeClose(sharpen::FileHandle handle) noexcept;

    public:
        explicit PosixNetStreamChannel(sharpen::FileHandle handle);

        virtual ~PosixNetStreamChannel() noexcept;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                sharpen::Future<std::size_t> &future) override;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,
                                   std::uint64_t size,
                                   std::uint64_t offset,
                                   sharpen::Future<std::size_t> &future) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,
                                   sharpen::Future<std::size_t> &future) override;

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) override;

        virtual void ConnectAsync(const sharpen::IEndPoint &endpoint,
                                  sharpen::Future<void> &future) override;

        virtual void Listen(std::uint16_t queueLength) override;

        virtual void PollReadAsync(sharpen::Future<void> &future) override;

        virtual void PollWriteAsync(sharpen::Future<void> &future) override;

        virtual void Cancel() noexcept override;
    };
}   // namespace sharpen

#endif
#endif
