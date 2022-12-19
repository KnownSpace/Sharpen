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
        sharpen::IFiberScheduler *scheduler{sharpen::GetFiberScheduler()};
        assert(scheduler != nullptr);
        scheduler->Launch(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

    template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,_Args...>::Value>>
    inline void LaunchSpecial(std::size_t stackSize,_Fn &&fn,_Args &&...args)
    {
        sharpen::IFiberScheduler *scheduler{sharpen::GetFiberScheduler()};
        assert(scheduler != nullptr);
        scheduler->LaunchSpecial(stackSize,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
    }

    template<typename _Fn,typename ..._Args,typename _Result = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
    inline sharpen::AwaitableFuturePtr<_Result> Async(_Fn &&fn,_Args &&...args)
    {
        auto future = sharpen::MakeAwaitableFuture<_Result>();
        sharpen::IFiberScheduler *scheduler{sharpen::GetFiberScheduler()};
        assert(scheduler != nullptr);
        scheduler->Invoke(*future,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        return future;
    }

    template<typename _Fn,typename ..._Args,typename _Result = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
    inline sharpen::AwaitableFuturePtr<_Result> AsyncSpecial(std::size_t stackSize,_Fn &&fn,_Args &&...args)
    {
        auto future = sharpen::MakeAwaitableFuture<_Result>();
        sharpen::IFiberScheduler *scheduler{sharpen::GetFiberScheduler()};
        assert(scheduler != nullptr);
        scheduler->InvokeSpecial(stackSize,*future,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        return future;
    }
}

#endif
