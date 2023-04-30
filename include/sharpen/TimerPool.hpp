#pragma once
#ifndef _SHARPEN_TIMERPOOL_HPP
#define _SHARPEN_TIMERPOOL_HPP

#include "IEventLoopGroup.hpp"
#include "ITimerPool.hpp"
#include <mutex>
#include <vector>

namespace sharpen
{
    class TimerPool
        : public sharpen::ITimerPool
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = TimerPool;
        using TimerMaker = sharpen::TimerPtr (*)(sharpen::IEventLoopGroup *);

        static constexpr std::size_t reservedSize_{32};

        sharpen::SpinLock lock_;
        std::vector<sharpen::TimerPtr> timers_;
        TimerMaker maker_;
        sharpen::IEventLoopGroup *loopGroup_;

        inline sharpen::TimerPtr MakeTimer() const
        {
            if (this->maker_)
            {
                return this->maker_(this->loopGroup_);
            }
            return sharpen::MakeTimer(*this->loopGroup_);
        }

    public:
        explicit TimerPool(sharpen::IEventLoopGroup &loopGroup)
            : TimerPool(loopGroup, static_cast<std::size_t>(0))
        {
        }

        TimerPool(sharpen::IEventLoopGroup &loopGroup, std::size_t reserveCount)
            : TimerPool(loopGroup, nullptr, reserveCount)
        {
        }

        TimerPool(sharpen::IEventLoopGroup &loopGroup, TimerMaker maker)
            : TimerPool(loopGroup, maker, 0)
        {
        }

        TimerPool(sharpen::IEventLoopGroup &loopGroup, TimerMaker maker, std::size_t reserveCount);

        ~TimerPool() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::TimerPtr GetTimer() override;

        virtual void Reserve(std::size_t size) override;

        virtual void ReturnTimer(sharpen::TimerPtr &&timer) noexcept override;
    };
}   // namespace sharpen

#endif