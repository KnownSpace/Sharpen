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
        using MyFuture = sharpen::Future<std::size_t>;
        using MyFuturePtr = MyFuture*;
        using Mybase = sharpen::IFileChannel;
        using Self = sharpen::WinFileChannel;
        
        static void InitOverlapped(OVERLAPPED &ol,std::uint64_t offset);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event,std::uint64_t offset);

        void RequestWrite(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future);

        void RequestRead(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future);

    public:

        explicit WinFileChannel(sharpen::FileHandle handle);

        ~WinFileChannel() noexcept = default;

        virtual void WriteAsync(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual std::uint64_t GetFileSize() const override;

        virtual sharpen::FileMemory MapMemory(std::size_t size,std::uint64_t offset) override;

        virtual void Truncate() override;

        virtual void Truncate(std::uint64_t size) override;
        
        virtual void Flush() override;
    };
}

#endif
#endif
