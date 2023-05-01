#pragma once
#ifndef _SHARPEN_WRITELOGSRESULT_HPP
#define _SHARPEN_WRITELOGSRESULT_HPP

#include "Optional.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class WriteLogsResult {
    private:
        using Self = sharpen::WriteLogsResult;

        std::uint64_t lastIndex_;
        std::uint64_t beginIndex_;

    public:
        explicit WriteLogsResult(std::uint64_t lastIndex) noexcept;

        WriteLogsResult(std::uint64_t lastIndex, std::uint64_t beginIndex) noexcept;

        WriteLogsResult(const Self &other) noexcept = default;

        WriteLogsResult(Self &&other) noexcept;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~WriteLogsResult() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        bool GetStatus() const noexcept;

        std::uint64_t GetLastIndex() const noexcept;

        sharpen::Optional<std::uint64_t> LookupBeginIndex() const noexcept;

        std::size_t GetWrittenSize() const noexcept;
    };
}   // namespace sharpen

#endif