#include <sharpen/AsyncBarrier.hpp>

sharpen::AsyncBarrier::AsyncBarrier(sharpen::Uint32 counter)
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
            return;
        }
        this->waiters_.push_back(&future);
    }
    future.Await();
}

void sharpen::AsyncBarrier::Reset()
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    this->counter_ = this->beginCounter_;
}

void sharpen::AsyncBarrier::Notice() noexcept
{
    sharpen::AsyncBarrier::MyFuturePtr futurePtr;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        assert(this->counter_ != 0);
        this->counter_ -= 1;
        if(this->counter_ != 0 || this->waiters_.size() == 0)
        {
            return;
        }
        futurePtr = this->waiters_.front();
        this->waiters_.pop_front();
    }
    futurePtr->Complete();
}
