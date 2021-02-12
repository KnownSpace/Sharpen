#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "CoroutineEngine.hpp"
#include "Awaiter.hpp"

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
            ,awaiter_()
        {}
        
        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
            ,awaiter_(std::move(other.awaiter_))
        {}
        
        virtual ~AwaitableFuture() noexcept = default;

        auto Await() -> decltype(this->Get())
        {
            //enable coroutine function
            sharpen::InitThisThreadForCentralEngine();
            {
                sharpen::SpinLock &lock = this->GetLock();
                std::unique_lock<sharpen::SpinLock> _lock(lock);
                if (this->IsPending())
                {
                    //load current context
                    sharpen::ExecuteContextPtr *ctx = new sharpen::ExecuteContextPtr(std::move(sharpen::ExecuteContext::GetCurrentContext()));
                    sharpen::Awaiter &awaiter = this->awaiter_;
                    sharpen::LocalContextSwitchCallback = [this,&awaiter,&lock,ctx](){
                        std::unique_ptr<sharpen::ExecuteContextPtr> contextPtr(ctx);
                        std::unique_lock<sharpen::SpinLock> _lock(lock);
                        awaiter.Wait(std::move(*contextPtr));
                        bool completed = this->CompletedOrError();
                        _lock.unlock();
                        if (completed)
                        {
                            awaiter.Notify();
                        }
                    };
                    _lock.unlock();
                    sharpen::LocalSchedulerContext->Switch();
                }
            }
            return this->Get();
        }
        
        Self &operator=(Self &&other) noexcept
        {
            MyBase::operator=(std::move(other));
            this->awaiter_ = std::move(other.awaiter_);
            return *this;
        }

    protected:

        virtual void ExecuteCallback() override
        {
            MyBase::ExecuteCallback();
            this->awaiter_.Notify();
        }

    private:

        sharpen::Awaiter awaiter_;
    };
  
    template<typename _T>
    using AwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;

    template<typename _T>
    inline sharpen::AwaitableFuturePtr<_T> MakeAwaitableFuture()
    {
        sharpen::AwaitableFuturePtr<_T> future = std::make_shared<sharpen::AwaitableFuture<_T>>();
        return std::move(future);
    }
}

#endif
