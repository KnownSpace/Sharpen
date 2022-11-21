#include <sharpen/AsyncNagleBarrier.hpp>

#include <cassert>

void sharpen::AsyncNagleBarrier::ResetWithoutLock() noexcept
{
    if(this->model_ == sharpen::BarrierModel::Boundaried && this->currentCount_ >= this->count_)
    {
        this->currentCount_ -= this->count_;
    }
    else
    {
        this->currentCount_ = 0;
    }
}

void sharpen::AsyncNagleBarrier::TimeoutNotice(sharpen::Future<bool> &future) noexcept
{
    bool timeout{future.Get()};
    if(timeout)
    {
        MyFuturePtr futurePtr{nullptr};
        std::size_t count{0};
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            this->timerStarted_ = false;
            if(this->waiters_.empty())
            {
                return;
            }
            futurePtr = this->waiters_.back();
            this->waiters_.pop_back();
            count = this->currentCount_;
            if(this->model_ == sharpen::BarrierModel::Boundaried && count > this->count_)
            {
                count = this->count_;
            }
            this->ResetWithoutLock();
        }
        futurePtr->Complete(count);
    }
}

void sharpen::AsyncNagleBarrier::StartTimer()
{
    if(!this->timerStarted_)
    {
        this->timerStarted_ = true;
        this->timeoutFuture_.Reset();
        this->timeoutFuture_.SetCallback(std::bind(&Self::TimeoutNotice,this,std::placeholders::_1));
        this->timer_->WaitAsync(this->timeoutFuture_,this->timeout_);
    }
}

void sharpen::AsyncNagleBarrier::StopTimer()
{
    if(this->timerStarted_)
    {
        this->timer_->Cancel();
        this->timerStarted_ = false;
    }
}

std::size_t sharpen::AsyncNagleBarrier::WaitAsync()
{
    MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if(this->currentCount_ >= this->count_)
        {
            std::size_t currentCount{this->currentCount_};
            if(this->model_ == sharpen::BarrierModel::Boundaried && currentCount > this->count_)
            {
                currentCount = this->count_;
            }
            this->ResetWithoutLock();
            return currentCount;
        }
        this->waiters_.emplace_back(&future);
        this->StartTimer();
    }
    return future.Await();
}

void sharpen::AsyncNagleBarrier::Notify(std::size_t count) noexcept
{
    assert(count != 0);
    MyFuturePtr future{nullptr};
    std::size_t currentCount{0};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        this->currentCount_ += count;
        if(this->currentCount_ >= this->count_)
        {
            //notice
            if(!this->waiters_.empty())
            {
                future = this->waiters_.back();
                this->waiters_.pop_back();
                currentCount = this->currentCount_;
                if(this->model_ == sharpen::BarrierModel::Boundaried && currentCount > this->count_)
                {
                    currentCount = this->count_;
                }
            }
            //close timer
            this->StopTimer();
        }
    }
    if(future)
    {
        future->Complete(currentCount);
    }
}

void sharpen::AsyncNagleBarrier::Reset() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock{this->lock_};
    this->ResetWithoutLock();
}