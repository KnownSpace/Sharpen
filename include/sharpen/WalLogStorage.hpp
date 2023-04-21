#pragma once
#ifndef _SHARPEN_WALLOGSTORAGE_HPP
#define _SHARPEN_WALLOGSTORAGE_HPP

#include "AsyncRwLock.hpp"
#include "IFileChannel.hpp"
#include "ILogStorage.hpp"
#include "LogEntries.hpp"
#include <map>

namespace sharpen
{
    class WalLogStorage
        : public sharpen::ILogStorage
        , public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::WalLogStorage;
        using Logs = std::map<std::uint64_t, sharpen::ByteBuffer>;

        constexpr static std::uint8_t writeTag_{0};

        constexpr static std::uint8_t removeTag_{1};

        constexpr static std::size_t limitFactor_{3};

        std::string name_;
        std::string tempName_;
        sharpen::FileChannelPtr channel_;
        sharpen::IEventLoopGroup *loopGroup_;
        std::unique_ptr<sharpen::AsyncRwLock> lock_;
        Logs logs_;
        std::uint64_t offset_;
        std::size_t contentSize_;

        bool Insert(std::uint64_t index, sharpen::ByteBuffer log);

        bool Erase(std::uint64_t index) noexcept;

        void Load();

        std::size_t ComputeContentSize() const noexcept;

        void RebuildFile();

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(
            std::uint64_t index) const override;

        virtual void NviWrite(std::uint64_t index, sharpen::ByteSlice log) override;

        virtual void NviDropUntil(std::uint64_t index) noexcept override;

        virtual void NviTruncateFrom(std::uint64_t index) override;

        virtual void NviWriteBatch(std::uint64_t beginIndex, sharpen::LogEntries entries) override;

    public:
        WalLogStorage(std::string name);

        WalLogStorage(sharpen::IEventLoopGroup &loopGroup, std::string name);

        WalLogStorage(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        virtual ~WalLogStorage() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual std::uint64_t GetLastIndex() const override;
    };
}   // namespace sharpen

#endif