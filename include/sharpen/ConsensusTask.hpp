#pragma once
#ifndef _SHARPEN_CONSENSUSTASK_HPP
#define _SHARPEN_CONSENSUSTASK_HPP

#include "ConsensusStatus.hpp"
#include "Future.hpp"
#include "Noncopyable.hpp"
#include <cstdint>


namespace sharpen {
    class ConsensusTask : public sharpen::Noncopyable {
    private:
        using Self = sharpen::ConsensusTask;

        std::uint64_t index_;
        sharpen::Future<sharpen::ConsensusStatus> *future_;

    public:
        ConsensusTask(std::uint64_t index,
                      sharpen::Future<sharpen::ConsensusStatus> &future) noexcept;

        ConsensusTask(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~ConsensusTask() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        bool Vaild() const noexcept;

        void Complete(sharpen::ConsensusStatus status);

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
    };
}   // namespace sharpen

#endif