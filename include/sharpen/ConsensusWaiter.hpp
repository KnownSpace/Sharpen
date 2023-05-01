#pragma once
#ifndef _SHARPEN_CONSENSUSWAITER_HPP
#define _SHARPEN_CONSENSUSWAITER_HPP

#include "Future.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace sharpen {
    class ConsensusWaiter {
    private:
        using Self = sharpen::ConsensusWaiter;

        std::uint64_t index_;
        sharpen::Future<std::uint64_t> *future_;

    public:
        ConsensusWaiter() noexcept;

        ConsensusWaiter(std::uint64_t index, sharpen::Future<std::uint64_t> &future) noexcept;

        ConsensusWaiter(const Self &other) noexcept = default;

        ConsensusWaiter(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~ConsensusWaiter() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        std::int32_t CompareWith(const Self &other) const noexcept;

        inline bool operator==(const Self &other) const noexcept {
            return this->CompareWith(other) == 0;
        }

        inline bool operator!=(const Self &other) const noexcept {
            return this->CompareWith(other) != 0;
        }

        inline bool operator<(const Self &other) const noexcept {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>(const Self &other) const noexcept {
            return this->CompareWith(other) > 0;
        }

        inline bool operator>=(const Self &other) const noexcept {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept {
            return this->CompareWith(other) <= 0;
        }

        inline sharpen::Future<std::uint64_t> &Future() noexcept {
            assert(this->future_ != nullptr);
            return *this->future_;
        }

        inline const sharpen::Future<std::uint64_t> &Future() const noexcept {
            assert(this->future_ != nullptr);
            return *this->future_;
        }

        inline std::uint64_t GetIndex() const noexcept {
            return this->index_;
        }
    };
}   // namespace sharpen

#endif