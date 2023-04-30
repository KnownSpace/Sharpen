#pragma once
#ifndef _SHARPEN_LOGBATCH_HPP
#define _SHARPEN_LOGBATCH_HPP

#include "ByteBuffer.hpp"
#include <vector>

namespace sharpen
{
    class LogBatch
    {
    private:
        using Self = sharpen::LogBatch;

        static constexpr std::size_t reverseCount_{5};

        std::vector<sharpen::ByteBuffer> entires_;

    public:
        LogBatch() = default;

        LogBatch(const Self &other) = default;

        LogBatch(Self &&other) noexcept = default;

        inline Self &operator=(const Self &other)
        {
            if (this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~LogBatch() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Append(sharpen::ByteBuffer log);

        std::size_t GetSize() const noexcept;

        sharpen::ByteBuffer &Get(std::size_t index) noexcept;

        const sharpen::ByteBuffer &Get(std::size_t index) const noexcept;

        void Reverse(std::size_t size);

        bool Empty() const noexcept;
    };
}   // namespace sharpen

#endif