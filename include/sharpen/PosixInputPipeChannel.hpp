#pragma once
#ifndef _SHARPEN_POSIXINPUTPIPECHANNEL_HPP
#define _SHARPEN_POSIXINPUTPIPECHANNEL_HPP

#include "SystemMacro.hpp"
#ifdef SHARPEN_IS_NIX

#include <functional>

#include "IInputPipeChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "PosixIoReader.hpp"

#define SHARPEN_HAS_POSIXINPUTPIPE

namespace sharpen
{
    class PosixInputPipeChannel
        : public sharpen::IInputPipeChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IInputPipeChannel;
        using Callback = std::function<void(ssize_t)>;
        using Self = sharpen::PosixInputPipeChannel;

        sharpen::PosixIoReader reader_;
        bool readable_;

        void HandleRead();

        void DoRead();

        void TryRead(char *buf, std::size_t bufSize, Callback cb);

        void RequestRead(char *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future);

        static void CompleteReadCallback(sharpen::EventLoop *loop,
                                         sharpen::Future<std::size_t> *future,
                                         ssize_t size) noexcept;

        void DoSafeClose(sharpen::ErrorCode err, sharpen::ChannelPtr keepalive) noexcept;

        void SafeClose(sharpen::FileHandle handle) noexcept;

    public:
        explicit PosixInputPipeChannel(sharpen::FileHandle handle);

        virtual ~PosixInputPipeChannel();

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen

#endif
#endif