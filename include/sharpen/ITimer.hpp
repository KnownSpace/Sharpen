#pragma once
#ifndef _SHARPEN_ITIMER_HPP
#define _SHARPEN_ITIMER_HPP

#include <chrono>
#include <functional>
#include <memory>

#include "AwaitableFuture.hpp"

namespace sharpen
{
    class EventLoop;

    class EventEngine;

    class ITimer
    {
    private:
        using Self = sharpen::ITimer;
        using WaitFuture = sharpen::Future<void>;

    public:
        ITimer() = default;

        ITimer(const Self &other) = default;

        ITimer(Self &&other) noexcept = default;

        virtual ~ITimer() noexcept = default;

        virtual void WaitAsync(WaitFuture &future,sharpen::Uint64 waitMs) = 0;

        virtual void Cancel() = 0;

        template<typename _Rep,typename _Period>
        inline void WaitAsync(WaitFuture &future,const std::chrono::duration<_Rep,_Period> &time)
        {
            this->WaitAsync(future,time/std::chrono::milliseconds(1));
        }

        template<typename _Rep,typename _Period>
        void Await(const std::chrono::duration<_Rep,_Period> &time)
        {
            sharpen::AwaitableFuture<void> future;
            this->WaitAsync(future,time);
            future.Await();
        }
    };

    using TimerPtr = std::shared_ptr<sharpen::ITimer>;

    extern sharpen::TimerPtr MakeTimer(sharpen::EventLoop &loop);

    extern sharpen::TimerPtr MakeTimer(sharpen::EventEngine &engine);
}

#endif