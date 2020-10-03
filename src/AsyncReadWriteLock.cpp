#include <sharpen/AsyncReadWriteLock.hpp>

sharpen::AsyncReadWriteLock::AsyncReadWriteLock()
	:state_(sharpen::ReadWriteLockState::Free)
	,readWaiters_()
	,writeWaiters_()
	,lock_()
	,readers_(0)
{}

void sharpen::AsyncReadWriteLock::ReadLock(sharpen::AsyncReadWriteLock::Function &&callback)
{
	{
		std::unique_lock<sharpen::SpinLock> lock(this->lock_);
		if (this->state_ == sharpen::ReadWriteLockState::UniquedWriting)
		{
			this->readWaiters_.push_back(std::move(callback));
			return;
		}
		this->readers_ += 1;
		this->state_ = sharpen::ReadWriteLockState::SharedReading;
	}
	callback();
}

void sharpen::AsyncReadWriteLock::WriteLock(sharpen::AsyncReadWriteLock::Function &&callback)
{
	{
		std::unique_lock<sharpen::SpinLock> lock(this->lock_);
		if (this->state_ != sharpen::ReadWriteLockState::Free)
		{
			this->writeWaiters_.push_back(std::move(callback));
			return;
		}
		this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
	}
	callback();
}

void sharpen::AsyncReadWriteLock::NoticeAllReaders(sharpen::AsyncReadWriteLock::List &newList)
{
    this->state_ = sharpen::ReadWriteLockState::SharedReading;
    this->readers_ += this->readWaiters_.size();
    std::swap(this->readWaiters_,newList);
}

void sharpen::AsyncReadWriteLock::NoticeWriter(sharpen::AsyncReadWriteLock::Function &callback)
{
    callback = std::move(this->writeWaiters_.front());
	this->writeWaiters_.pop_front();
    this->state_ = sharpen::ReadWriteLockState::UniquedWriting;
}

void sharpen::AsyncReadWriteLock::WriteUnlock()
{
	std::unique_lock<sharpen::SpinLock> lock(this->lock_);
	if (!this->writeWaiters_.empty())
	{
		sharpen::AsyncReadWriteLock::Function callback;
        this->NoticeWriter(callback);
		lock.unlock();
		callback();
		return;
	}
	else if (!this->readWaiters_.empty())
	{
		sharpen::AsyncReadWriteLock::List list;
        this->NoticeAllReaders(list);
        lock.unlock();
        for (auto begin = std::begin(list),end = std::end(list);begin != end;++begin)
        {
            (*begin)();
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
        sharpen::AsyncReadWriteLock::Function callback;
		this->NoticeWriter(callback);
        lock.unlock();
        callback();
		return;
	}
    this->state_ = sharpen::ReadWriteLockState::Free;
}