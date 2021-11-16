#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "EventEngine.hpp"

namespace sharpen
{
    template <typename _Result>
    class AwaitableFuture : public sharpen::Future<_Result>
    {
    private:
        using MyBase = sharpen::Future<_Result>;
        using Self = sharpen::AwaitableFuture<_Result>;

        sharpen::IFiberScheduler *scheduler_;
        sharpen::FiberPtr pendingFiber_;
        sharpen::FiberPtr awaiter_;

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
                fiber = std::move(this->pendingFiber_);
            }
            this->scheduler_->Schedule(std::move(fiber));
        }

    public:
        AwaitableFuture()
            :AwaitableFuture(&sharpen::EventEngine::GetEngine())
        {}

        explicit AwaitableFuture(sharpen::IFiberScheduler *scheduler)
            :MyBase()
            ,scheduler_(scheduler)
            ,pendingFiber_(nullptr)
            ,awaiter_(nullptr)
        {}

        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
            ,scheduler_(other.scheduler_)
            ,pendingFiber_(std::move(other.pendingFiber_))
            ,awaiter_(std::move(other.awaiter_))
        {
            other.scheduler_ = nullptr;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                MyBase::operator=(std::move(other));
                this->scheduler_ = other.scheduler_;
                this->pendingFiber_ = std::move(other.pendingFiber_);
                this->awaiter_ = std::move(other.awaiter_);
                other.scheduler_ = nullptr;
            }
            return *this;
        }

        virtual ~AwaitableFuture() noexcept = default;

        void PreparePending()
        {
            this->pendingFiber_ = std::move(sharpen::Fiber::GetCurrentFiber());
        }

        void PrepareAwaiter()
        {
            this->awaiter_ = std::move(this->pendingFiber_);
        }

        void NotifyAwaiter()
        {
            sharpen::FiberPtr fiber;
            {
                std::unique_lock<sharpen::SpinLock> lock(this->GetCompleteLock());
                fiber = std::move(this->awaiter_);
            }
            if(fiber)
            {
                this->scheduler_->Schedule(std::move(fiber));
            }
        }

        void WaitAsync()
        {
            //this thread is not a processer
            if (!this->scheduler_->IsProcesser())
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
                    this->scheduler_->SetSwitchCallback(std::bind(&sharpen::AwaitableFuture<_Result>::NotifyIfCompleted,this));
                    this->scheduler_->SwitchToProcesserFiber();
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