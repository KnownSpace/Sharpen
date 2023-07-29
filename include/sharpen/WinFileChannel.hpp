#pragma once
#ifndef _SHARPEN_WINFILECHANNEL_HPP
#define _SHARPEN_WINFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_WINFILE

#include "AwaitableFuture.hpp"   // IWYU pragma: keep
#include "IFileChannel.hpp"
#include "IocpSelector.hpp"   // IWYU pragma: keep
#include "Optional.hpp"
#include <mutex>

namespace sharpen {
    class WinFileChannel
        : public sharpen::IFileChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using MyFuture = sharpen::Future<std::size_t>;
        using MyFuturePtr = MyFuture *;
        using Mybase = sharpen::IFileChannel;
        using Self = sharpen::WinFileChannel;

        static void InitOverlapped(OVERLAPPED &ol, std::uint64_t offset);

        static sharpen::Optional<bool> supportSparseFile_;

        bool sparesFile_;

        bool syncWrite_;

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event, std::uint64_t offset);

        void RequestWrite(const char *buf,
                          std::size_t bufSize,
                          std::uint64_t offset,
                          sharpen::Future<std::size_t> *future);

        void RequestRead(char *buf,
                         std::size_t bufSize,
                         std::uint64_t offset,
                         sharpen::Future<std::size_t> *future);

        void RequestFlushAsync(sharpen::Future<void> *future);

        void RequestAllocate(sharpen::Future<std::size_t> *future,std::uint64_t offset, std::size_t size);

        void RequestDeallocate(sharpen::Future<std::size_t> *future,std::uint64_t offset, std::size_t size);

    public:
        explicit WinFileChannel(sharpen::FileHandle handle, bool syncWrite);

        virtual ~WinFileChannel() noexcept = default;

        static bool SupportSparseFile(const char *rootName) noexcept;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                std::uint64_t offset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                std::uint64_t offset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               std::uint64_t offset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               std::uint64_t offset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual std::uint64_t GetFileSize() const override;

        virtual sharpen::FileMemory MapMemory(std::size_t size, std::uint64_t offset) override;

        virtual void Truncate() override;

        virtual void Truncate(std::uint64_t size) override;

        virtual void Flush() override;

        virtual void FlushAsync(sharpen::Future<void> &future) override;

        void EnableSparesFile();

        virtual void AllocateAsync(sharpen::Future<std::size_t> &future, std::uint64_t offset, std::size_t size) override;

        virtual void DeallocateAsync(sharpen::Future<std::size_t> &future, std::uint64_t offset, std::size_t size) override;

        virtual std::size_t GetPath(SHARPEN_OUT char *path, std::size_t size) const override;
    };
}   // namespace sharpen

#endif
#endif
