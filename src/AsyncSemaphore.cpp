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

void sharpen::AsyncSemaphore::Unlock(sharpen::Uint32 count) noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->waiters_.empty())
    {
        this->counter_ += count;
        return;
    }
    if(count >= this->waiters_.size())
    {
        sharpen::AsyncSemaphore::List futures;
        this->waiters_.swap(futures);
        this->count_ += (count - this->waiters_.size());
        lock.unlock();
        for(auto begin = futures.begin,end = futures.end();begin != end; ++begin)
        {
            (*begin)->Complete();
        }
    }
    else
    {
        auto begin = this->waiters_.begin();
        auto end = begin;
        for(;count != 0;--count)
        {
            ++end;
        }
        sharpen::AsyncSemaphore::List futures(std::make_move_iterator(begin),std::make_move_iterator(end));
        lock.unlock();
        begin = futures.begin();
        end = futures.end();
        while(begin != end)
        {
            (*begin)->Complete();
            ++begin;
        }
    }
}
