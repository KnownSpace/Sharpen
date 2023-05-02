#pragma once
#ifndef _SHARPEN_YIELDOPS_HPP
#define _SHARPEN_YIELDOPS_HPP

#include "Fiber.hpp"
#include "IFiberScheduler.hpp"
#include <cassert>
#include <thread>
#include <utility>

namespace sharpen {
    class YieldCycleCallback {
    private:
        using Self = YieldCycleCallback;

        sharpen::FiberPtr fiber_;

    public:
        YieldCycleCallback(sharpen::FiberPtr fiber) noexcept
            : fiber_(std::move(fiber)) {
        }

        YieldCycleCallback(const Self &other) = default;

        YieldCycleCallback(Self &&other) noexcept = default;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept {
            if (this != std::addressof(other)) {
                this->fiber_ = std::move(other.fiber_);
            }
            return *this;
        }

        ~YieldCycleCallback() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline void operator()() noexcept {
            if (this->fiber_) {
                sharpen::IFiberScheduler *scheduler{this->fiber_->GetScheduler()};
                assert(scheduler);
                scheduler->ScheduleSoon(std::move(this->fiber_));
            }
        }
    };

    void YieldCycle();
}   // namespace sharpen

#endif