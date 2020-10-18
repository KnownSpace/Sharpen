#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "CoroutineEngine.hpp"
#include "AwaitableFutureSwitchCallback.hpp"

namespace sharpen
{
    template<typename _Result>
    class AwaitableFuture:public sharpen::Future<_Result>
    {
    public:

        auto Await() -> decltype(this->Get())
        {
            //enable coroutine function
            sharpen::InitThisThreadForCentralEngine();
            //not complete yet
            if (!this->CompletedOrError())
            {
                //load current context
                std::unique_ptr<sharpen::ExecuteContext> contextPtr(std::move(sharpen::ExecuteContext::GetCurrentContext()));
                auto &context = *contextPtr;
                //set switch callback
                sharpen::LocalContextSwitchCallback.reset(new sharpen::AwaitableFutureSwitchCallback<_Result>(std::move(contextPtr),*this));
                //switch context
                sharpen::LocalEngineContext->Switch(context);
            }
            return this->Get();
        }
    };
  
    template<typename _T>
    using SharedAwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;

    template<typename _T>
    sharpen::SharedAwaitableFuturePtr<_T> MakeSharedAwaitableFuture()
    {
        sharpen::SharedAwaitableFuturePtr<_T> future = std::make_shared<sharpen::AwaitableFuture<_T>>();
        if (!future)
        {
            throw std::bad_alloc();
        }
        return future;
    }
}

#endif
