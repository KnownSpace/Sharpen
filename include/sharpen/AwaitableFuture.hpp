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
        using MyBase = sharpen::Future<_Result>;
        using Self = sharpen::AwaitableFuture<_Result>;
    public:

        AwaitableFuture()
            :MyBase()
        {}
        
        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
        {}
        
        virtual ~AwaitableFuture() = default;

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
        
        Self &operator=(Self &&other) noexcept
        {
            MyBase::operator=(std::move(other));
            return *this;
        }
    };
  
    template<typename _T>
    using SharedAwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;

    template<typename _T>
    inline sharpen::SharedAwaitableFuturePtr<_T> MakeSharedAwaitableFuture()
    {
        sharpen::SharedAwaitableFuturePtr<_T> future = std::make_shared<sharpen::AwaitableFuture<_T>>();
        return std::move(future);
    }
}

#endif
