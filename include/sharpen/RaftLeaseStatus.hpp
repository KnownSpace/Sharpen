#pragma once
#ifndef _SHARPEN_RAFTLEASESTATUS_HPP
#define _SHARPEN_RAFTLEASESTATUS_HPP

#include <cstdint>
#include <utility>
#include <cstddef>

namespace sharpen {
    class RaftLeaseStatus {
    private:
        using Self = sharpen::RaftLeaseStatus;

        std::uint64_t leaseRound_;
        std::size_t ackCount_;
    public:
        RaftLeaseStatus() noexcept;

        RaftLeaseStatus(const Self &other) noexcept;

        RaftLeaseStatus(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftLeaseStatus() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void NextRound() noexcept;

        std::uint64_t GetRound() const noexcept;

        void OnAck() noexcept;

        std::size_t GetAckCount() const noexcept;
    };
}   // namespace sharpen

#endif