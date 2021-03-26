#pragma once
#ifndef _SHARPEN_WINFILECHANNEL_HPP
#define _SHARPEN_WINFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_WINFILE

#include "IFileChannel.hpp"
#include "AwaitableFuture.hpp"
#include "IocpSelector.hpp"

namespace sharpen
{
    class WinFileChannel:public sharpen::IFileChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<sharpen::Size>;
        using MyFuturePtr = MyFuture*;
        using Mybase = sharpen::IFileChannel;
        
        static void InitOverlapped(OVERLAPPED &ol,sharpen::Uint64 offset);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event,sharpen::Uint64 offset);

    public:

        WinFileChannel(sharpen::FileHandle handle);

        ~WinFileChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual sharpen::Uint64 GetFileSize() const override;
    };
}

#endif
#endif
