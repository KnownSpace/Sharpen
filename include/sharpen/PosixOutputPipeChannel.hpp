#pragma once
#ifndef _SHARPEN_POSIXOUTPUTCHANNEL_HPP
#define _SHARPEN_POSIXOUTPUTCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#include <functional>

#include "IOutputPipeChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "PosixIoWriter.hpp"

#define SHARPEN_HAS_POSIXOUTPUTPIPE

namespace sharpen {
    class PosixOutputPipeChannel
        : public sharpen::IOutputPipeChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Mybase = sharpen::IOutputPipeChannel;
        using Callback = std::function<void(ssize_t)>;
        using Self = sharpen::PosixOutputPipeChannel;

        sharpen::PosixIoWriter writer_;
        bool writeable_;
        bool peerClosed_;

        void DoWrite();

        void HandleWrite();

        void TryWrite(const char *buf, std::size_t bufSize, Callback cb);

        void RequestWrite(const char *buf,
                          std::size_t bufSize,
                          sharpen::Future<std::size_t> *future);

        static void CompleteWriteCallback(sharpen::EventLoop *loop,
                                          sharpen::Future<std::size_t> *future,
                                          ssize_t size) noexcept;

        void DoSafeClose(sharpen::ErrorCode err, sharpen::ChannelPtr keepalive) noexcept;

        void SafeClose(sharpen::FileHandle handle) noexcept;

    public:
        explicit PosixOutputPipeChannel(sharpen::FileHandle handle);

        virtual ~PosixOutputPipeChannel() noexcept;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                sharpen::Future<std::size_t> &future) override;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen

#endif
#endif