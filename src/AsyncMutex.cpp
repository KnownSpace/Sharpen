#include <sharpen/AsyncMutex.hpp>

sharpen::AsyncMutex::AsyncMutex()
    :locked_(false)
    ,waiters_()
    ,lock_()
{}

void sharpen::AsyncMutex::Lock(sharpen::AsyncMutex::Function &&callback)
{
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->locked_)
        {
            this->waiters_.push_back(std::move(callback));
            return;
        }
        this->locked_ = true;
    }
    callback();
}

sharpen::SharedFuturePtr<void> sharpen::AsyncMutex::LockAsync()
{
    sharpen::SharedFuturePtr<void> future = sharpen::MakeSharedFuturePtr<void>();
    this->Lock([future]()
    {
        future->Complete();
    });
    return future;
}

void sharpen::AsyncMutex::Unlock()
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->waiters_.empty())
    {
        this->locked_ = false;
        return;
    }
    sharpen::AsyncMutex::Function callback = std::move(this->waiters_.front());
    this->waiters_.pop_front();
    lock.unlock();
    callback();
}