#include <sharpen/AsyncSemaphore.hpp>

sharpen::AsyncSemaphore::AsyncSemaphore(sharpen::Uint32 count)
    :waiters_()
    ,lock_()
    ,counter_(count)
{}

void sharpen::AsyncSemaphore::LockAsync()
{
    sharpen::AsyncSemaphore::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (!this->NeedWait())
        {
            this->counter_ -= 1;
            return;
        }
        this->waiters_.push_back(&future);
    }
    future.Await();
}

bool sharpen::AsyncSemaphore::NeedWait() const
{
    return this->counter_ == 0;
}

void sharpen::AsyncSemaphore::Unlock() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->waiters_.empty())
    {
        this->counter_ += 1;
        return;
    }
    sharpen::AsyncSemaphore::MyFuturePtr futurePtr = this->waiters_.front();
    this->waiters_.pop_front();
    lock.unlock();
    futurePtr->Complete();
}
