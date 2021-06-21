#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "Awaiter.hpp"
#include "FiberScheduler.hpp"

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
            ,pendingFiber_(nullptr)
        {}
        
        AwaitableFuture(Self &&other) noexcept
            :MyBase(std::move(other))
            ,awaiter_(std::move(other.awaiter_))
            ,pendingFiber_(std::move(other.pendingFiber_))
        {}
        
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
                sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
                std::unique_lock<MyBase> lock(*this);
                //this thread is not a processer
                if (!sharpen::FiberScheduler::IsProcesser())
                {
                    while (this->IsPending())
                    {
                        lock.unlock();
                        scheduler.ProcessOnce(std::chrono::milliseconds(100));
                        lock.lock();
                    }
                }
                //this thread is a processer
                else
                {
                    if (this->IsPending())
                    {
                        lock.unlock();
                        this->pendingFiber_ = sharpen::Fiber::GetCurrentFiber();
                        scheduler.SwitchToProcesser([this]() mutable
                        {
                            bool completed;
                            {
                                std::unique_lock<sharpen::Future<_Result>> lock(*this);
                                completed = this->CompletedOrError();
                                this->PrepareAwaiter();
                            }
                            if (completed)
                            {
                                this->NotifyAwaiter();
                            }
                        });
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
