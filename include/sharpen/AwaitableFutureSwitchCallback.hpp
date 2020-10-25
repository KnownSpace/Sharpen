#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURESWITCHCALLBACK_HPP
#define _SHARPEN_AWAITABLEFUTURESWITCHCALLBACK_HPP

#include <memory>

#include "Awaiter.hpp"
#include "Future.hpp"
#include "ExecuteContext.hpp"
#include "IContextSwitchCallback.hpp"

namespace sharpen
{
    template<typename _T>
    class AwaitableFutureSwitchCallback:public sharpen::Noncopyable,public sharpen::IContextSwitchCallback
    {
    private:
        using MyAwaiter = sharpen::Awaiter<_T>;
        using ContextPtr = std::unique_ptr<sharpen::ExecuteContext>;
        using MyFuture = sharpen::Future<_T>;
        using Self = sharpen::AwaitableFutureSwitchCallback<_T>;

        MyAwaiter *awaiterPtr_;
        MyFuture &futureRef_;
    public:
        AwaitableFutureSwitchCallback(ContextPtr &&context,MyFuture &future)
            :awaiterPtr_(new MyAwaiter(std::move(context)))
            ,futureRef_(future)
        {}

        virtual void Run() noexcept override
        {
            MyAwaiter *awaiter = this->awaiterPtr_;
            futureRef_.SetCallback([awaiter](MyFuture &future){
                std::unique_ptr<MyAwaiter> tmp(awaiter);
                (*tmp)(future);
            });
        }

        virtual ~AwaitableFutureSwitchCallback() noexcept = default;

        Self &operator=(Self &&other) noexcept
        {
            this->awaiterPtr_ = other.awaiterPtr_;
            this->futureRef_ = other.futureRef_;
            return *this;
        }
    };
}

#endif
