#pragma once
#ifndef _SHARPEN_RAFTPREVOTERECORD_HPP
#define _SHARPEN_RAFTPREVOTERECORD_HPP

#include <cstddef>
#include <cstdint>
#include <set>

namespace sharpen
{
    class RaftPrevoteRecord
    {
    private:
        using Self = sharpen::RaftPrevoteRecord;

        std::uint64_t term_;
        std::set<std::uint64_t> votes_;

    public:
        RaftPrevoteRecord() = default;

        RaftPrevoteRecord(const Self &other) = default;

        RaftPrevoteRecord(Self &&other) noexcept;

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

        ~RaftPrevoteRecord() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        void Flush() noexcept;

        void Flush(std::uint64_t term) noexcept;

        std::uint64_t GetVotes() const noexcept;

        void Receive(std::uint64_t actorId);
    };
}   // namespace sharpen

#endif