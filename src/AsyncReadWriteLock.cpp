#include <cassert>
#include <sharpen/AsyncReadWriteLock.hpp>

sharpen::AsyncReadWriteLock::AsyncReadWriteLock()
    :state_(sharpen::ReadWriteLockState::Free)
    ,readWaiters_()
    ,writeWaiters_()
    ,lock_()
    ,readers_(0)
{}

sharpen::ReadWriteLockState sharpen::AsyncReadWriteLock::LockRead()
{
    sharpen::AsyncReadWriteLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->state_ != sharpen::ReadWriteLockState::UniquedWriting)
        {
            this->readers_ += 1;
            auto old{this->state_};
            this->state_ = sharpen::ReadWriteLockState::SharedReading;
            return old;
        }
        this->readWaiters_.push_back(&future);
    }
    return future.Await();
}

bool sharpen::AsyncReadWriteLock::TryLockRead()
{
    sharpen::ReadWriteLockState status{sharpen::ReadWriteLockState::Free};
    return this->TryLockRead(status);
}

bool sharpen::AsyncReadWriteLock::TryLockRead(sharpen::ReadWriteLockState &status)
{
    {
        if(!this->lock_.TryLock())
        {
            return false;
        }
        std::unique_lock<sharpen::SpinLock> lock{this->lock_,std::adopt_lock};
        if (this->state_ != sharpen::ReadWriteLockState::UniquedWriting)
        {
            status = this->state_;
            this->readers_ += 1;
            this->state_ = sharpen::ReadWriteLockState::SharedReading;
            return true;
        }
        return false;
    }
}

sharpen::ReadWriteLockState sharpen::AsyncReadWriteLock::LockWrite()
{
    sharpen::AsyncReadWriteLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->state_ == sharpen::ReadWriteLockState::Free)
        {
            this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
            return sharpen::ReadWriteLockState::Free;
        }
        this->writeWaiters_.push_back(&future);	
    }
    return future.Await();
}

bool sharpen::AsyncReadWriteLock::TryLockWrite()
{
    sharpen::ReadWriteLockState status{sharpen::ReadWriteLockState::Free};
    return this->TryLockWrite(status);
}

bool sharpen::AsyncReadWriteLock::TryLockWrite(sharpen::ReadWriteLockState &prevStatus)
{
    {
        if (!this->lock_.TryLock())
        {
            return false;
        }
        std::unique_lock<sharpen::SpinLock> lock{this->lock_,std::adopt_lock};
        if (this->state_ == sharpen::ReadWriteLockState::Free)
        {
            this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
            prevStatus = sharpen::ReadWriteLockState::Free;
            return true;
        }
        return false;
    }
}

void sharpen::AsyncReadWriteLock::WriteUnlock() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (!this->writeWaiters_.empty())
    {
        sharpen::AsyncReadWriteLock::MyFuturePtr futurePtr = this->writeWaiters_.back();
        this->writeWaiters_.pop_back();
        this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
        lock.unlock();
        futurePtr->Complete(sharpen::ReadWriteLockState::UniquedWriting);
        return;
    }
    else if (!this->readWaiters_.empty())
    {
        sharpen::AsyncReadWriteLock::Waiters waiters;
        std::swap(waiters,this->readWaiters_);
        this->readers_ = static_cast<sharpen::Uint32>(waiters.size());
        this->state_ = sharpen::ReadWriteLockState::SharedReading;
        lock.unlock();
        for (auto begin = std::begin(waiters),end = std::end(waiters);begin != end;++begin)
        {
            (*begin)->Complete(sharpen::ReadWriteLockState::UniquedWriting);
        }
        return;
    }
    this->state_ = sharpen::ReadWriteLockState::Free;
}

void sharpen::AsyncReadWriteLock::ReadUnlock() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    this->readers_ -= 1;
    if (this->readers_ != 0)
    {
        return;
    }
    if (!this->writeWaiters_.empty())
    {
        sharpen::AsyncReadWriteLock::MyFuturePtr futurePtr = this->writeWaiters_.back();
        this->writeWaiters_.pop_back();
        this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
        lock.unlock();
        futurePtr->Complete(sharpen::ReadWriteLockState::SharedReading);
        return;
    }
    this->state_ = sharpen::ReadWriteLockState::Free;
}

void sharpen::AsyncReadWriteLock::Unlock() noexcept
{
    assert(this->state_ != sharpen::ReadWriteLockState::Free);
    if(this->state_ == sharpen::ReadWriteLockState::UniquedWriting)
    {
        this->WriteUnlock();
    }
    else if(this->state_ == sharpen::ReadWriteLockState::SharedReading)
    {
        this->ReadUnlock();
    }
}
