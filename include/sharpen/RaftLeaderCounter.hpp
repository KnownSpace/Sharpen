#pragma once
#ifndef _SHARPEN_RAFTLEADERCOUNTER_HPP
#define _SHARPEN_RAFTLEADERCOUNTER_HPP

#include <atomic>
#include <cstdint>
#include <utility>


namespace sharpen {
    class RaftLeaderCounter {
    private:
        using Self = sharpen::RaftLeaderCounter;

        std::atomic_uint64_t count_;

    public:
        RaftLeaderCounter() noexcept;

        RaftLeaderCounter(const Self &other) noexcept;

        RaftLeaderCounter(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftLeaderCounter() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        std::uint64_t GetCurrentCount() const noexcept;

        bool TryStepUp(std::uint64_t prevCount) noexcept;

        void StepDown() noexcept;
    };
}   // namespace sharpen

#endif