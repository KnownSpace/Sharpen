#pragma once
#ifndef _SHARPEN_ELECTION_TIMER
#define _SHARPEN_ELECTION_TIMER

#include <random>
#include <chrono>

#include "ITimer.hpp"
#include "AwaitableFuture.hpp"

namespace sharpen
{
    class ElectionTimer:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        sharpen::TimerPtr timer_;
        sharpen::Future<bool> future_;
        sharpen::Future<bool> *notify_;
        std::minstd_rand random_;
        std::uniform_int_distribution<sharpen::Uint32> distrubution_;

        void DoWaitAsync();

        void Notify(sharpen::Future<bool> &future);
    public:
        template<typename _Rep1,typename _Period1,typename _Rep2,typename _Period2>
        ElectionTimer(sharpen::TimerPtr timer,const std::chrono::duration<_Rep1,_Period1> &minWait,const std::chrono::duration<_Rep2,_Period2> &maxWait)
            :ElectionTimer(std::move(timer),minWait,maxWait,std::random_device{}())
        {}

        template<typename _Rep1,typename _Period1,typename _Rep2,typename _Period2>
        ElectionTimer(sharpen::TimerPtr timer,const std::chrono::duration<_Rep1,_Period1> &minWait,const std::chrono::duration<_Rep2,_Period2> &maxWait,sharpen::Uint32 seed)
            :timer_(timer)
            ,future_()
            ,notify_(nullptr)
            ,random_(seed)
            ,distrubution_(minWait/std::chrono::milliseconds(1),maxWait/std::chrono::milliseconds(1))
        {}

        void WaitAsync(sharpen::Future<bool> &future);

        void Reset();

        inline bool Await()
        {
            sharpen::AwaitableFuture<bool> future;
            this->WaitAsync(future);
            return future.Await();
        }

        void Cancel();

        ~ElectionTimer() noexcept = default;
    };
}

#endif