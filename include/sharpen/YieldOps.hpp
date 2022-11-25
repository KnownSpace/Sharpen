#pragma once
#ifndef _SHARPEN_YIELDOPS_HPP
#define _SHARPEN_YIELDOPS_HPP

#include <thread>
#include <utility>
#include <cassert>

#include "Fiber.hpp"
#include "IFiberScheduler.hpp"

namespace sharpen
{
    class YieldCycleCallback
    {
    private:
        using Self = YieldCycleCallback;

        sharpen::FiberPtr fiber_;
        sharpen::IFiberScheduler *scheduler_;
    public:

        YieldCycleCallback(sharpen::FiberPtr fiber,sharpen::IFiberScheduler *scheduler) noexcept
            :fiber_(std::move(fiber))
            ,scheduler_(scheduler)
        {}

        YieldCycleCallback(const Self &other) = default;

        YieldCycleCallback(Self &&other) noexcept
            :fiber_(std::move(other.fiber_))
            ,scheduler_(other.scheduler_)
        {
            other.scheduler_ = nullptr;
        }

        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->fiber_ = std::move(other.fiber_);
                this->scheduler_ = other.scheduler_;
                other.scheduler_ = nullptr;
            }
            return *this;
        }

        ~YieldCycleCallback() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void operator()() noexcept
        {
            if(this->fiber_)
            {
                assert(this->scheduler_);
                sharpen::IFiberScheduler *scheduler{nullptr};
                std::swap(scheduler,this->scheduler_);
                scheduler->ScheduleSoon(std::move(this->fiber_));
            }
        }
    };

    void YieldCycle();
}

#endif