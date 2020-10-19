#include <cassert>
#include <sharpen/AsyncReadWriteLock.hpp>

sharpen::AsyncReadWriteLock::AsyncReadWriteLock()
    :state_(sharpen::ReadWriteLockState::Free)
    ,readWaiters_()
    ,writeWaiters_()
    ,lock_()
    ,readers_(0)
{}

void sharpen::AsyncReadWriteLock::LockReadAsync()
{
    sharpen::AsyncReadWriteLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->state_ != sharpen::ReadWriteLockState::UniquedWriting)
        {
            this->readers_ += 1;
            this->state_ = sharpen::ReadWriteLockState::SharedReading;
			return;
		}
		this->readWaiters_.push_back(&future);
	}
	future.Await();
}

void sharpen::AsyncReadWriteLock::LockWriteAsync()
{
    sharpen::AsyncReadWriteLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->state_ == sharpen::ReadWriteLockState::Free)
        {
            this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
            return;
		}
        this->writeWaiters_.push_back(&future);	
    }
    future.Await();
}

void sharpen::AsyncReadWriteLock::WriteUnlock()
{
	std::unique_lock<sharpen::SpinLock> lock(this->lock_);
	if (!this->writeWaiters_.empty())
	{
		sharpen::AsyncReadWriteLock::MyFuturePtr futurePtr = thus->writeWaiters_.front();
        this->writeWaiters_.pop();
        this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
		lock.unlock();
		futurePtr->Complete();
		return;
	}
	else if (!this->readWaiters_.empty())
	{
		sharpen::AsyncReadWriteLock::List list;
        std::swap(list,this->readWaiters_);
        this->readers_ = list.size();
        this->state_ = sharpen::ReadWriteLockState::SharedReading;
        lock.unlock();
        for (auto begin = std::begin(list),end = std::end(list);begin != end;++begin)
        {
            (*begin)->Complete();
        }
        return;
	}
	this->state_ = sharpen::ReadWriteLockState::Free;
}

void sharpen::AsyncReadWriteLock::ReadUnlock()
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->readers_ != 1)
    {
        this->readers_ -= 1;
        return;
    }
    if (!this->writeWaiters_.empty())
	{
        sharpen::AsyncReadWriteLock::MyFuturePtr futurePtr = this->writeWaiters_.front();
        this->writeWaiters_.pop();
        this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
        lock.unlock();
        futurePtr->Complete();
		return;
	}
    this->state_ = sharpen::ReadWriteLockState::Free;
}

void sharpen::AsyncReadWriteLock::Unlock() noexcept()
{
    assert(this->state_ != sharpen::ReadWriteLockState::Free);
    if(this->state_ == sharpen::ReadWriteLockState::UniquedWriting)
    {
        this->WriteUnlock();
    }
    else if(this->state_ == sharpen::ReadWriteLockState::SharedRead)
    {
        this->ReadUnlock();
    }
}
