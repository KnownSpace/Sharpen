#pragma once
#include "sharpen/ByteSlice.hpp"
#include "sharpen/LogEntries.hpp"
#include <cstddef>
#include <utility>
#ifndef _SHARPEN_ILOGSTORAGE_HPP
#define _SHARPEN_ILOGSTORAGE_HPP

#include "ByteBuffer.hpp"
#include "LogEntries.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class ILogStorage
    {

    private:
        using Self = sharpen::ILogStorage;

    protected:
        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(std::uint64_t index) const = 0;

        virtual void NviWrite(std::uint64_t index, sharpen::ByteSlice log) = 0;

        virtual void NviWriteBatch(std::uint64_t beginIndex, sharpen::LogEntries entires) = 0;

        virtual void NviDropUntil(std::uint64_t index) noexcept = 0;

        virtual void NviTruncateFrom(std::uint64_t index) = 0;

    public:
        constexpr static std::uint64_t noneIndex{0};

        ILogStorage() noexcept = default;

        ILogStorage(const Self &other) noexcept = default;

        ILogStorage(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~ILogStorage() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual std::uint64_t GetLastIndex() const = 0;

        inline sharpen::Optional<sharpen::ByteBuffer> Lookup(std::uint64_t index) const
        {
            if (index != noneIndex)
            {
                return this->NviLookup(index);
            }
            return sharpen::EmptyOpt;
        }

        inline void Write(std::uint64_t index, sharpen::ByteSlice log)
        {
            assert(index != noneIndex);
            assert(!log.Empty());
            this->NviWrite(index, log);
        }

        inline void Write(std::uint64_t index, const sharpen::ByteBuffer &log)
        {
            this->Write(index, log.GetSlice());
        }

        inline void WriteBatch(std::uint64_t beginIndex, sharpen::LogEntries entires)
        {
            assert(beginIndex != noneIndex);
            if (!entires.Empty())
            {
                this->NviWriteBatch(beginIndex, std::move(entires));
            }
        }

        inline void DropUntil(std::uint64_t index) noexcept
        {
            if (index <= this->GetLastIndex())
            {
                this->NviDropUntil(index);
            }
        }

        inline void TruncateFrom(std::uint64_t index)
        {
            if (index <= this->GetLastIndex())
            {
                this->NviTruncateFrom(index);
            }
        }
    };
}   // namespace sharpen

#endif