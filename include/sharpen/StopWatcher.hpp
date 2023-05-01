#pragma once
#ifndef _SHARPEN_STOPWATCHER_HPP
#define _SHARPEN_STOPWATCHER_HPP

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <cstddef>
#include <cstdint>
#include <ctime>

namespace sharpen {
    struct StopWatcher
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        std::clock_t begin_;
        std::clock_t end_;

    public:
        StopWatcher() = default;

        ~StopWatcher() noexcept = default;

        inline void Begin() noexcept {
            this->begin_ = std::clock();
        }

        inline void Stop() noexcept {
            this->end_ = std::clock();
        }

        inline std::clock_t Compute() noexcept {
            return this->end_ - this->begin_;
        }

        static inline constexpr std::clock_t TimeUnitPerSecond() noexcept {
            return CLOCKS_PER_SEC;
        }
    };
}   // namespace sharpen

#endif