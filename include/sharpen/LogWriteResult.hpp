#pragma once
#ifndef _SHARPEN_LOGWRITERESULT_HPP
#define _SHARPEN_LOGWRITERESULT_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "Optional.hpp"

namespace sharpen
{
    class LogWriteResult
    {
    private:
        using Self = sharpen::LogWriteResult;
    
        std::uint64_t lastIndex_;
        std::uint64_t beginIndex_;
    public:
    
        explicit LogWriteResult(std::uint64_t lastIndex) noexcept;

        LogWriteResult(std::uint64_t lastIndex,std::uint64_t beginIndex) noexcept;
    
        LogWriteResult(const Self &other) noexcept = default;
    
        LogWriteResult(Self &&other) noexcept;
    
        Self &operator=(const Self &other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~LogWriteResult() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        bool GetStatus() const noexcept;

        std::uint64_t GetLastIndex() const noexcept;

        sharpen::Optional<std::uint64_t> LookupBeginIndex() const noexcept;

        std::size_t GetWrittenSize() const noexcept;
    };    
}

#endif