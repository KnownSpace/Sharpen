#pragma once
#ifndef _SHARPEN_POSIXFILECHANNEL_HPP
#define _SHARPEN_POSIXFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXFILE

#include "IFileChannel.hpp"
#include "IoUringQueue.hpp"

#ifdef SHARPEN_HAS_IOURING
#include "IoUringStruct.hpp"
#endif

namespace sharpen {
    class PosixFileChannel
        : public sharpen::IFileChannel
        , public sharpen::Noncopyable {
    private:
        using MyBase = sharpen::IFileChannel;
        using Self = sharpen::PosixFileChannel;

        void NormalRead(char *buf,
                        std::size_t bufSize,
                        std::uint64_t offset,
                        sharpen::Future<std::size_t> *future);

        void NormalWrite(const char *buf,
                         std::size_t bufSize,
                         std::uint64_t offset,
                         sharpen::Future<std::size_t> *future);

        void DoRead(char *buf,
                    std::size_t bufSize,
                    std::uint64_t offset,
                    sharpen::Future<std::size_t> *future);

        void DoWrite(const char *buf,
                     std::size_t bufSize,
                     std::uint64_t offset,
                     sharpen::Future<std::size_t> *future);

#ifdef SHARPEN_HAS_IOURING

        sharpen::IoUringStruct *InitStruct(void *buf,
                                           std::size_t bufSize,
                                           sharpen::Future<std::size_t> *future);

        sharpen::IoUringStruct *InitStruct(sharpen::Future<void> *future);

        sharpen::IoUringQueue *queue_;
#endif

        void NormalFlush(sharpen::Future<void> *future);

        void DoFlushAsync(sharpen::Future<void> *future);

        bool syncWrite_;

    public:
        explicit PosixFileChannel(sharpen::FileHandle handle, bool syncWrite);

        virtual ~PosixFileChannel() noexcept = default;

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

        virtual void Register(sharpen::EventLoop &loop) override;

        virtual std::uint64_t GetFileSize() const override;

        virtual sharpen::FileMemory MapMemory(std::size_t size, std::uint64_t offset) override;

        virtual void Truncate() override;

        virtual void Truncate(std::uint64_t size) override;

        virtual void Flush() override;

        virtual void FlushAsync(sharpen::Future<void> &future) override;

        virtual void Allocate(std::uint64_t offset, std::size_t size) override;

        virtual void Deallocate(std::uint64_t offset, std::size_t size) override;
    };

}   // namespace sharpen

#endif
#endif
