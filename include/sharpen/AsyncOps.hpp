#pragma once
#ifndef _SHARPEN_ASYNCOPS_HPP

#include <type_traits>
#include <thread>

#include "AwaitableFuture.hpp"
#include "FiberScheduler.hpp"

namespace sharpen
{
    template<typename _Fn,typename ..._Args>
    inline void Launch(_Fn &&fn,_Args &&...args)
    {
        sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
        sharpen::FiberPtr fiber = sharpen::Fiber::MakeFiber(16*1024,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        scheduler.Schedule(std::move(fiber));
    }

    template<typename _Fn,typename _Result>
    struct AsyncHelper
    {
        static void RunAndSetFuture(_Fn &fn,sharpen::Future<_Result> &future)
        {
            try
            {
                auto &&val = fn();
                future.Complete(std::move(val));
            }
            catch(const std::exception&)
            {
                future.Fail(std::current_exception());
            }
        }
    };

    template<typename _Fn>
    struct AsyncHelper<_Fn,void>
    {
        static void RunAndSetFuture(_Fn &fn,sharpen::Future<void> &future)
        {
            try
            {
                fn();
                future.Complete();
            }
            catch(const std::exception&)
            {
                future.Fail(std::current_exception());
            }
        }
    };
    

    template<typename _Fn,typename ..._Args,typename _Result = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
    inline sharpen::AwaitableFuturePtr<_Result> Async(_Fn &&fn,_Args &&...args)
    {
        auto future = sharpen::MakeAwaitableFuture<_Result>();
        std::function<_Result()> func = std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        sharpen::Launch([func,future]() mutable
        {
            sharpen::AsyncHelper<std::function<_Result()>,_Result>::RunAndSetFuture(func,*future);
        });
        return future;
    }

    extern void Delay();

    template <class _Rep, class _Period>
    inline void Delay(std::chrono::duration<_Rep,_Period> &time)
    {
        sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
        scheduler.ProcessOnce(time);
    }
}

#endif
