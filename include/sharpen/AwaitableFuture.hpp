#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "Awaiter.hpp"
#include "EventEngine.hpp"

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
            ,scheduler_(&sharpen::EventEngine::GetEngine())
            ,awaiter_(this->scheduler_)
            ,pendingFiber_(nullptr)
        {}
        
        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
            ,scheduler_(other.scheduler_)
            ,awaiter_(std::move(other.awaiter_))
            ,pendingFiber_(std::move(other.pendingFiber_))
        {
            other.scheduler_ = nullptr;
        }
        
        virtual ~AwaitableFuture() noexcept = default;

        void PrepareAwaiter()
        {
            this->awaiter_.Wait(std::move(this->pendingFiber_));
        }

        void NotifyAwaiter()
        {
            this->awaiter_.Notify();
        }

        auto Await() -> decltype(this->Get())
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->GetCompleteLock());
                //this thread is not a processer
                if (!this->scheduler_->IsProcesser())
                {
                    lock.unlock();
                    this->Wait();
                }
                //this thread is a processer
                else
                {
                    if (this->IsPending())
                    {
                        lock.unlock();
                        this->pendingFiber_ = sharpen::Fiber::GetCurrentFiber();
                        this->scheduler_->SetSwitchCallback([this]() mutable
                        {
                            bool completed;
                            {
                                std::unique_lock<sharpen::SpinLock> lock(this->GetCompleteLock());
                                completed = this->CompletedOrError();
                                this->PrepareAwaiter();
                            }
                            if (completed)
                            {
                                this->NotifyAwaiter();
                            }
                        });
                        this->scheduler_->SwitchToProcesserFiber();
                    }
                }
            }
            return this->Get();
        }
        
        Self &operator=(Self &&other) noexcept
        {
            MyBase::operator=(std::move(other));
            this->awaiter_ = std::move(other.awaiter_);
            this->pendingFiber_ = std::move(other.awaiter_);
            return *this;
        }

    protected:

        virtual void ExecuteCallback() override
        {
            MyBase::ExecuteCallback();
            this->NotifyAwaiter();
        }

    private:

        sharpen::IFiberScheduler *scheduler_;
        sharpen::Awaiter awaiter_;
        sharpen::FiberPtr pendingFiber_;
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
