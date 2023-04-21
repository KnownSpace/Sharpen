#pragma once
#ifndef _SHARPEN_RAFTLEADERRECORD_HPP
#define _SHARPEN_RAFTLEADERRECORD_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen
{
    class RaftLeaderRecord
    {
    private:
        using Self = sharpen::RaftLeaderRecord;

        std::atomic_uint64_t term_;
        std::atomic_uint64_t leaderId_;

    public:
        RaftLeaderRecord() noexcept;

        RaftLeaderRecord(std::uint64_t term, std::uint64_t leaderId) noexcept;

        RaftLeaderRecord(const Self &other) noexcept;

        RaftLeaderRecord(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept
        {
            if (this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftLeaderRecord() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        std::pair<std::uint64_t, std::uint64_t> GetRecord() const noexcept;

        void Flush(std::uint64_t term, std::uint64_t leaderId) noexcept;
    };
}   // namespace sharpen

#endif