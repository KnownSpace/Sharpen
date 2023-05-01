#pragma once
#ifndef _SHARPEN_RAFTELECTIONRECORD_HPP
#define _SHARPEN_RAFTELECTIONRECORD_HPP

#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class RaftElectionRecord {
    private:
        using Self = sharpen::RaftElectionRecord;

        std::uint64_t term_;
        std::uint64_t votes_;

    public:
        RaftElectionRecord() noexcept;

        RaftElectionRecord(std::uint64_t term, std::uint64_t votes) noexcept;

        RaftElectionRecord(const Self &other) = default;

        RaftElectionRecord(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftElectionRecord() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept {
            return this->term_;
        }

        inline void Flush(std::uint64_t term) noexcept {
            this->term_ = term;
            this->votes_ = 0;
        }

        inline std::uint64_t GetVotes() const noexcept {
            return this->votes_;
        }

        inline void SetVotes(std::uint64_t votes) noexcept {
            this->votes_ = votes;
        }
    };
}   // namespace sharpen

#endif