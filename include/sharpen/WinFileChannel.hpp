#pragma once
#ifndef _SHARPEN_WINFILECHANNEL_HPP
#define _SHARPEN_WINFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_WINFILE

#include <mutex>

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

        static void Closer(sharpen::FileHandle file,sharpen::FileHandle mapping) noexcept;

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event,sharpen::Uint64 offset);

        void DoInitFileMapping();

        void InitFileMapping();

        std::once_flag mappingFlag_;

        sharpen::FileHandle mappingHandle_;

    public:

        explicit WinFileChannel(sharpen::FileHandle handle);

        ~WinFileChannel() noexcept = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual sharpen::Uint64 GetFileSize() const override;

        virtual sharpen::FileMemory MapMemory(sharpen::Size size,sharpen::Uint64 offset) override;
    };
}

#endif
#endif
