#include <sharpen/AsyncBarrier.hpp>
#include <cassert>

sharpen::AsyncBarrier::AsyncBarrier(std::uint64_t counter)
    :counter_(counter)
    ,waiters_()
    ,beginCounter_(counter)
{}

void sharpen::AsyncBarrier::WaitAsync()
{
    sharpen::AsyncBarrier::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->counter_ == 0)
        {
            this->ResetWithoutLock();
            return;
        }
        this->waiters_.push_back(&future);
    }
    future.Await();
}

void sharpen::AsyncBarrier::ResetWithoutLock() noexcept
{
    this->counter_ = this->beginCounter_;
}

void sharpen::AsyncBarrier::Reset()
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    this->ResetWithoutLock();
}

void sharpen::AsyncBarrier::Notice() noexcept
{
    MyFuturePtr futurePtr{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        assert(this->counter_ != 0);
        this->counter_ -= 1;
        if(this->counter_ != 0 || this->waiters_.empty())
        {
            return;
        }
        futurePtr = this->waiters_.back();
        this->waiters_.pop_back();
        this->ResetWithoutLock();
    }
    futurePtr->Complete();
}