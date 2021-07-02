#pragma once
#ifndef _SHARPEN_ASYNCOPS_HPP

#include <type_traits>
#include <thread>

#include "AwaitableFuture.hpp"
#include "AsyncHelper.hpp"

namespace sharpen
{
    template<typename _Fn,typename ..._Args>
    inline void Launch(_Fn &&fn,_Args &&...args)
    {
        sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
        engine.Launch(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

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
}

#endif
