#include <sharpen/AsyncNagleBarrier.hpp>

void sharpen::AsyncNagleBarrier::ResetWithoutLock() noexcept
{
    this->currentCount_ = 0;
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
            if (this->waiters_.empty())
            {
                return;
            }
            futurePtr = this->waiters_.back();
            this->waiters_.pop_back();
            count = this->currentCount_;
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
    this->timer_->Cancel();
}

std::size_t sharpen::AsyncNagleBarrier::WaitAsync()
{
    MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if(this->count_ == this->currentCount_)
        {
            this->ResetWithoutLock();
            return this->count_;
        }
        this->waiters_.emplace_back(&future);
        this->StartTimer();
    }
    return future.Await();
}

void sharpen::AsyncNagleBarrier::Notice()
{
    MyFuturePtr future{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        assert(this->currentCount_ < this->count_);
        this->currentCount_ += 1;
        if(this->currentCount_ == this->count_)
        {
            //notice
            if(!this->waiters_.empty())
            {
                future = this->waiters_.back();
                this->waiters_.pop_back();
            }
            //close timer
            this->StopTimer();
            this->timerStarted_ = false;
        }
    }
    if(future)
    {
        future->Complete(this->count_);
    }
}