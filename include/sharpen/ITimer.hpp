#pragma once
#ifndef _SHARPEN_ITIMER_HPP
#define _SHARPEN_ITIMER_HPP

#include <chrono>
#include <functional>

#include "Future.hpp"

namespace sharpen
{
    class ITimer
    {
    private:
        using Self = sharpen::ITimer;
        using WaitFuture = sharpen::Future<void>;
    protected:
        using Closer = std::function<void()>;

        Closer closer_;
    public:
        ITimer() = default;

        ITimer(const Self &other) = default;

        ITimer(Self &&other) noexcept = default;

        virtual ~ITimer() noexcept
        {
            if (this->closer_)
            {
                this->closer_();
            }
        }

        virtual void WaitAsync(WaitFuture &future);

        void Await();
    };
}

#endif