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
    class PosixInputPipeChannel:public sharpen::IInputPipeChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IInputPipeChannel;
        using Callback = std::function<void(ssize_t)>;

        sharpen::PosixIoReader reader_;
        bool readable_;

        void HandleRead();

        void DoRead();

        void TryRead(char *buf,sharpen::Size bufSize,Callback cb);

        void RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);

        static void CompleteReadCallback(sharpen::EventLoop *loop,sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept;
    public:
        explicit PosixInputPipeChannel(sharpen::FileHandle handle);

        virtual ~PosixInputPipeChannel() = default;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}

#endif
#endif