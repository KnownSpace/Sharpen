#pragma once
#ifndef _SHARPEN_TIMEOPS_HPP
#define _SHARPEN_TIMEOPS_HPP

#include <cassert>

#include "AwaitableFuture.hpp"
#include "TimerPool.hpp"
#include "TypeTraits.hpp"
#include "FutureCompletor.hpp"
#include "IEventLoopGroup.hpp"

namespace sharpen
{
    struct TimerHelper
    {
    private:
        using Maker = sharpen::TimerPtr(*)(sharpen::IEventLoopGroup*);

        static std::unique_ptr<sharpen::TimerPool> gobalTimerPool_;
        static std::once_flag flag_;

        static void InitTimerPool(sharpen::IEventLoopGroup *loopGroup,Maker maker);
    public:

        static sharpen::TimerPool &GetTimerPool(sharpen::IEventLoopGroup &loopGroup,Maker maker);
    };

    inline sharpen::TimerPool &GetGobalTimerPool()
    {
        sharpen::IEventLoopGroup *loopGroup{sharpen::GetLocalLoopGroup()};
        assert(loopGroup != nullptr);
        return sharpen::TimerHelper::GetTimerPool(*loopGroup,nullptr);   
    }

    template<typename _Rep,typename _Period>
    inline void Delay(const std::chrono::duration<_Rep,_Period> &time)
    {
        sharpen::TimerPool &pool{sharpen::GetGobalTimerPool()};
        sharpen::TimerPtr timer{pool.GetTimer()};
        timer->Await(time);
        pool.PutTimer(std::move(timer));
    }

    template<typename _T>
    struct AwaitForHelper
    {
        static void AwaitForCallback(sharpen::Future<_T> &future,sharpen::TimerPtr timer)
        {
            (void)future;
            assert(timer != nullptr);
            timer->Cancel();
        }
    };
    
    enum class AwaitForResult
    {
        CompletedOrError,
        Timeout
    };

    template<typename _T,typename _Rep,typename _Period>
    inline sharpen::AwaitForResult AwaitFor(sharpen::Future<_T> &future,sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &time)
    {
        sharpen::AwaitableFuture<bool> waiter;
        timer->WaitAsync(waiter,time);
        future.SetCallback(std::bind(&AwaitForHelper<_T>::AwaitForCallback,std::placeholders::_1,timer));
        if(waiter.Await())
        {
            return sharpen::AwaitForResult::Timeout;
        }
        return sharpen::AwaitForResult::CompletedOrError;
    }
}

#endif