#include <sharpen/AsyncSemaphore.hpp>

sharpen::AsyncSemaphore::AsyncSemaphore(sharpen::Uint32 count)
    :waiters_()
    ,lock_()
    ,counter_(count)
{}

void sharpen::AsyncSemaphore::Lock(sharpen::AsyncSemaphore::Function &&callback)
{
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->NeedWait())
        {
            this->waiters_.push_back(std::move(callback));
            return;
        }
        this->counter_ -= 1;
    }
    callback();
}

bool sharpen::AsyncSemaphore::NeedWait() const
{
    return this->counter_ == 0;
}

void sharpen::AsyncSemaphore::Unlock()
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->waiters_.empty())
    {
        this->counter_ += 1;
        return;
    }
    sharpen::AsyncSemaphore::Function callback = std::move(this->waiters_.front());
    this->waiters_.pop_front();
    lock.unlock();
    callback();
}

sharpen::SharedFuturePtr<void> sharpen::AsyncSemaphore::LockAsync()
{
    sharpen::SharedFuturePtr<void> future = sharpen::MakeSharedFuturePtr<void>();
    this->Lock([future]()
    {
        future->Complete();
    });
    return future;
}