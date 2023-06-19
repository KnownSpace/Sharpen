#pragma once
#ifndef _SHARPEN_LINUXSIGNALFDCHANNEL_HPP
#define _SHARPEN_LINUXSIGNALFDCHANNEL_HPP

#include "SignalFd.hpp"   // IWYU pragma: keep

#ifdef SHARPEN_HAS_SIGNALFD

#include "ISignalChannel.hpp"
#include "SystemError.hpp"
#include <deque>

namespace sharpen {
    class LinuxSignalFdChannel
        : public sharpen::ISignalChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::LinuxSignalFdChannel;
        using Callback = std::function<void(ssize_t)>;
        using Base = sharpen::ISignalChannel;
        struct ReadTask {
            Callback cb;
            char *buf;
            std::size_t bufSize;
        };
        using Tasks = std::deque<ReadTask>;

        std::uint64_t bufMap_;
        bool readable_;
        bool peerClosed_;
        Tasks tasks_;

        void DoCancel(sharpen::ErrorCode err);

        void DoSafeClose(sharpen::ErrorCode err, sharpen::ChannelPtr keepalive) noexcept;

        void SafeClose(sharpen::FileHandle handle) noexcept;

        std::uint8_t PopSignal() noexcept;

        void HandleRead();

        void DoRead();

        void TryRead(char *buf, std::size_t bufSize, Callback cb);

        void RequestRead(char *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future);

        static void CompleteReadCallback(sharpen::EventLoop *loop,
                                         sharpen::Future<std::size_t> *future,
                                         ssize_t size) noexcept;

    public:
        LinuxSignalFdChannel(sharpen::FileHandle handle);

        virtual ~LinuxSignalFdChannel() noexcept;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void ReadAsync(sharpen::SignalBuffer &signals,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen

#endif
#endif