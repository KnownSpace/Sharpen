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

namespace sharpen
{
    class PosixOutputPipeChannel:public sharpen::IOutputPipeChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IOutputPipeChannel;
        using Callback = std::function<void(ssize_t)>;
        
        sharpen::PosixIoWriter writer_;
        bool writeable_;
        
        void DoWrite();

        void HandleWrite();

        void TryWrite(const char *buf,sharpen::Size bufSize,Callback cb);

        void RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future);
        
        static void CompleteWriteCallback(sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept;
    
    public:
        explicit PosixOutputPipeChannel(sharpen::FileHandle handle);

        virtual ~PosixOutputPipeChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}

#endif
#endif