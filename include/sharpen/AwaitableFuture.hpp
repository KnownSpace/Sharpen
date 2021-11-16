#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "EventEngine.hpp"

namespace sharpen
{
    struct Awaiter
    {
        sharpen::IFiberScheduler *scheduler_;
        sharpen::SpinLock awaitLock_;
        sharpen::FiberPtr awaiter_;
        sharpen::FiberPtr pendingFiber_;
    };

    template <typename _Result>
    class AwaitableFuture : public sharpen::Future<_Result>
    {
    private:
        using MyBase = sharpen::Future<_Result>;
        using Self = sharpen::AwaitableFuture<_Result>;
        using AwaiterPtr = std::unique_ptr<sharpen::Awaiter>;

        AwaiterPtr awaiter_;

        void NotifyIfCompleted()
        {
            sharpen::FiberPtr fiber;
            {
                std::unique_lock<sharpen::SpinLock> lock(this->GetCompleteLock());
                if(!this->CompletedOrError())
                {
                    this->PrepareAwaiter();
                    return;
                }
                fiber = std::move(this->awaiter_->pendingFiber_);
            }
            this->awaiter_->scheduler_->Schedule(std::move(fiber));
        }

    public:
        AwaitableFuture()
            :AwaitableFuture(&sharpen::EventEngine::GetEngine())
        {}

        explicit AwaitableFuture(sharpen::IFiberScheduler *scheduler)
            :MyBase()
            ,awaiter_(new Awaiter{})
        {
            if(!awaiter_)
            {
                throw std::bad_alloc();
            }
            this->awaiter_->scheduler_ = scheduler;
        }

        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
            ,awaiter_(std::move(other.awaiter_))
        {}

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                MyBase::operator=(std::move(other));
                this->awaiter_ = std::move(other.awaiter_);
            }
            return *this;
        }

        virtual ~AwaitableFuture() noexcept = default;

        void PreparePending()
        {
            this->awaiter_->pendingFiber_ = sharpen::Fiber::GetCurrentFiber();
        }

        void PrepareAwaiter()
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->awaiter_->awaitLock_);
                this->awaiter_->awaiter_ = std::move(this->awaiter_->pendingFiber_);
            }
        }

        void NotifyAwaiter()
        {
            sharpen::FiberPtr fiber;
            {
                std::unique_lock<sharpen::SpinLock> lock(this->awaiter_->awaitLock_);
                fiber = std::move(this->awaiter_->awaiter_);
            }
            if(fiber)
            {
                this->awaiter_->scheduler_->Schedule(std::move(fiber));
            }
        }

        void WaitAsync()
        {
            //this thread is not a processer
            if (!this->awaiter_->scheduler_->IsProcesser())
            {
                this->Wait();
            }
            //this thread is a processer
            else
            {
                if (this->IsPending())
                {
                    //this->pendingFiber_ = sharpen::Fiber::GetCurrentFiber();
                    this->PreparePending();
                    this->awaiter_->scheduler_->SetSwitchCallback(std::bind(&sharpen::AwaitableFuture<_Result>::NotifyIfCompleted,this));
                    this->awaiter_->scheduler_->SwitchToProcesserFiber();
                }
            }
        }

        auto Await() -> decltype(this->Get())
        {
            this->WaitAsync();
            return this->Get();
        }

    protected:
        virtual void ExecuteCallback() override
        {
            MyBase::ExecuteCallback();
            //must be last line
            //resume coroutine
            //and this maybe released
            this->NotifyAwaiter();
        }
    };

    template <typename _T>
    using AwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;

    template <typename _T>
    inline sharpen::AwaitableFuturePtr<_T> MakeAwaitableFuture()
    {
        return std::make_shared<sharpen::AwaitableFuture<_T>>();
    }

    template <typename _T>
    inline sharpen::AwaitableFuturePtr<_T> MakeAwaitableFuture(sharpen::IFiberScheduler *scheduler)
    {
        return std::make_shared<sharpen::AwaitableFuture<_T>>(scheduler);
    }
}

#endif