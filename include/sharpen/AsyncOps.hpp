#pragma once
#ifndef _SHARPEN_ASYNCOPS_HPP

#include <type_traits>

#include "CoroutineEngine.hpp"
#include "AwaitableFuture.hpp"

namespace sharpen
{
    template<typename _Fn,typename ..._Args>
    inline void Launch(_Fn &&fn,_Args &&...args)
    {
        sharpen::CentralEngine.PushTask(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

    template<typename _Fn,typename _Result>
    struct AsyncOpsHelper
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
    struct AsyncOpsHelper<_Fn,void>
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
            sharpen::AsyncOpsHelper<std::function<_Result()>,_Result>::RunAndSetFuture(func,*future);
        });
        return future;
    }

    extern void YieldTimeslice();
}

#endif
