#pragma once
#ifndef _SHARPEN_POSIXFILECHANNEL_HPP
#define _SHARPEN_POSIXFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXFILE

#include "IFileChannel.hpp"

namespace sharpen
{
    class PosixFileChannel:public sharpen::IFileChannel,public sharpen::Noncopyable
    {
    private:
        using MyBase = sharpen::IFileChannel;
        using Self = sharpen::PosixFileChannel;

        void DoRead(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future);

        void DoWrite(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future);
    public:

        explicit PosixFileChannel(sharpen::FileHandle handle);

        ~PosixFileChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void Register(sharpen::EventLoop *loop) override;

        virtual sharpen::Uint64 GetFileSize() const override;

        virtual sharpen::FileMemory MapMemory(sharpen::Size size,sharpen::Uint64 offset) override;
    };
    
}

#endif
#endif
