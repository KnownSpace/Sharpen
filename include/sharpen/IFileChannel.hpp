#pragma once
#ifndef _SHARPEN_IFILECHANNEL_HPP
#define _SHARPEN_IFILECHANNEL_HPP

#include "FileMemory.hpp"
#include "IAsyncRandomReadable.hpp"
#include "IAsyncRandomWritable.hpp"
#include "IChannel.hpp"
#include "SystemMacro.hpp"

namespace sharpen {
    class IFileChannel
        : public sharpen::IChannel
        , public sharpen::IAsyncRandomWritable
        , public sharpen::IAsyncRandomReadable {
    private:
        using Self = sharpen::IFileChannel;

    public:
#ifdef SHARPEN_IS_WIN
        constexpr static std::size_t AllocationGranularity = 64 * 1024;
#else
        constexpr static std::size_t AllocationGranularity = 4 * 1024;
#endif

        IFileChannel() noexcept = default;

        virtual ~IFileChannel() = default;

        IFileChannel(const Self &) = default;

        IFileChannel(Self &&other) noexcept = default;

        virtual std::uint64_t GetFileSize() const = 0;

        void ZeroMemoryAsync(sharpen::Future<std::size_t> &future,
                             std::size_t size,
                             std::uint64_t offset);

        std::size_t ZeroMemoryAsync(std::size_t size, std::uint64_t offset);

        inline std::size_t ZeroMemoryAsync(std::size_t size) {
            return this->ZeroMemoryAsync(size, 0);
        }

        virtual sharpen::FileMemory MapMemory(std::size_t size, std::uint64_t offset) = 0;

        virtual void Truncate() = 0;

        virtual void Truncate(std::uint64_t size) = 0;

        virtual void Flush() = 0;

        virtual void FlushAsync(sharpen::Future<void> &future) = 0;

        void FlushAsync();

        virtual void AllocateAsync(sharpen::Future<std::size_t> &future,std::uint64_t offset, std::size_t size) = 0;

        std::size_t AllocateAsync(std::uint64_t offset, std::size_t size);

        virtual void DeallocateAsync(sharpen::Future<std::size_t> &future,std::uint64_t offset, std::size_t size) = 0;

        std::size_t DeallocateAsync(std::uint64_t offset, std::size_t size);
    };

    using FileChannelPtr = std::shared_ptr<sharpen::IFileChannel>;

    extern sharpen::FileChannelPtr OpenFileChannel(const char *filename,
                                                   sharpen::FileAccessMethod access,
                                                   sharpen::FileOpenMethod open,
                                                   sharpen::FileIoMethod io);

    extern sharpen::FileChannelPtr OpenFileChannel(const char *filename,
                                                   sharpen::FileAccessMethod access,
                                                   sharpen::FileOpenMethod open);
}   // namespace sharpen

#endif
