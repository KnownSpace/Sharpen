#pragma once
#ifndef _SHARPEN_ASYNCOPS_HPP

#include <cassert>

#include "AwaitableFuture.hpp"
#include "TimerPool.hpp"
#include "IteratorOps.hpp"
#include "TypeTraits.hpp"
#include "FutureCompletor.hpp"

namespace sharpen
{
    template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,_Args...>::Value>>
    inline void Launch(_Fn &&fn,_Args &&...args)
    {
        sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
        engine.Launch(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

    template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,_Args...>::Value>>
    inline void LaunchSpecial(std::size_t stackSize,_Fn &&fn,_Args &&...args)
    {
        sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
        engine.LaunchSpecial(stackSize,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

    template<typename _Fn,typename ..._Args,typename _Result = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
    inline sharpen::AwaitableFuturePtr<_Result> Async(_Fn &&fn,_Args &&...args)
    {
        auto future = sharpen::MakeAwaitableFuture<_Result>();
        sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
        engine.Invoke(*future,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        return future;
    }

    template<typename _Fn,typename ..._Args,typename _Result = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
    inline sharpen::AwaitableFuturePtr<_Result> AsyncSpecial(std::size_t stackSize,_Fn &&fn,_Args &&...args)
    {
        auto future = sharpen::MakeAwaitableFuture<_Result>();
        sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
        engine.InvokeSpecial(stackSize,*future,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        return future;
    }
}

#endif
