#pragma once
#ifndef _SHARPEN_ASYNCNAGLEBARRIER_HPP
#define _SHARPEN_ASYNCNAGLEBARRIER_HPP

#include <utility>
#include <chrono>

#include "ITimer.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class AsyncNagleBarrier:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::AsyncNagleBarrier;
        using MyFuture = sharpen::AwaitableFuture<std::size_t>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;
    
        std::chrono::milliseconds timeout_;
        std::size_t count_;
        std::size_t currentCount_;
        bool timerStarted_;
        sharpen::TimerPtr timer_;
        sharpen::Future<bool> timeoutFuture_;
        Waiters waiters_;
        sharpen::SpinLock lock_;

        void TimeoutNotice(sharpen::Future<bool> &future) noexcept;

        void StartTimer();

        void StopTimer();

        void ResetWithoutLock() noexcept;
    public:
    
        template<typename _Rep,typename _Period>
        AsyncNagleBarrier(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,std::size_t count)
            :timeout_(timeout)
            ,count_(count)
            ,currentCount_(0)
            ,timerStarted_(false)
            ,timer_(std::move(timer))
            ,timeoutFuture_()
            ,waiters_()
            ,lock_()
        {}
    
        ~AsyncNagleBarrier() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        std::size_t WaitAsync();

        void Notice();

        void Reset();
    };   
}

#endif