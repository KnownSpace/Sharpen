#include <sharpen/AsyncRwLock.hpp>

#include <cassert>

sharpen::AsyncRwLock::AsyncRwLock()
    : state_(sharpen::RwLockState::Free)
    , readWaiters_()
    , writeWaiters_()
    , lock_()
    , readers_(0)
{
}

sharpen::RwLockState sharpen::AsyncRwLock::LockRead()
{
    sharpen::AsyncRwLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if (this->state_ != sharpen::RwLockState::UniquedWriting && this->writeWaiters_.empty())
        {
            this->readers_ += 1;
            auto old{this->state_};
            this->state_ = sharpen::RwLockState::SharedReading;
            return old;
        }
        this->readWaiters_.push_back(&future);
    }
    return future.Await();
}

bool sharpen::AsyncRwLock::TryLockRead() noexcept
{
    sharpen::RwLockState status{sharpen::RwLockState::Free};
    (void)status;
    return this->TryLockRead(status);
}

bool sharpen::AsyncRwLock::TryLockRead(sharpen::RwLockState &status) noexcept
{
    {
        // we always get spin lock
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if (this->state_ != sharpen::RwLockState::UniquedWriting && this->writeWaiters_.empty())
        {
            status = this->state_;
            this->readers_ += 1;
            this->state_ = sharpen::RwLockState::SharedReading;
            return true;
        }
        return false;
    }
}

sharpen::RwLockState sharpen::AsyncRwLock::LockWrite()
{
    sharpen::AsyncRwLock::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->state_ == sharpen::RwLockState::Free)
        {
            this->state_ = sharpen::RwLockState::UniquedWriting;
            return sharpen::RwLockState::Free;
        }
        this->writeWaiters_.push_back(&future);
    }
    return future.Await();
}

bool sharpen::AsyncRwLock::TryLockWrite() noexcept
{
    sharpen::RwLockState status{sharpen::RwLockState::Free};
    return this->TryLockWrite(status);
}

bool sharpen::AsyncRwLock::TryLockWrite(sharpen::RwLockState &prevStatus) noexcept
{
    {
        // if we could not get spin lock
        // we return false
        if (!this->lock_.TryLock())
        {
            return false;
        }
        std::unique_lock<sharpen::SpinLock> lock{this->lock_, std::adopt_lock};
        if (this->state_ == sharpen::RwLockState::Free)
        {
            this->state_ = sharpen::RwLockState::UniquedWriting;
            prevStatus = sharpen::RwLockState::Free;
            return true;
        }
        return false;
    }
}

void sharpen::AsyncRwLock::WriteUnlock() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock{this->lock_};
    assert(this->state_ == sharpen::RwLockState::UniquedWriting);
    if (!this->writeWaiters_.empty())
    {
        sharpen::AsyncRwLock::MyFuturePtr futurePtr = this->writeWaiters_.back();
        this->writeWaiters_.pop_back();
        this->state_ = sharpen::RwLockState::UniquedWriting;
        lock.unlock();
        futurePtr->Complete(sharpen::RwLockState::UniquedWriting);
        return;
    }
    else if (!this->readWaiters_.empty())
    {
        sharpen::AsyncRwLock::Waiters waiters;
        std::swap(waiters, this->readWaiters_);
        this->readers_ = static_cast<std::uint32_t>(waiters.size());
        this->state_ = sharpen::RwLockState::SharedReading;
        lock.unlock();
        for (auto begin = std::begin(waiters), end = std::end(waiters); begin != end; ++begin)
        {
            (*begin)->Complete(sharpen::RwLockState::UniquedWriting);
        }
        return;
    }
    this->state_ = sharpen::RwLockState::Free;
}

void sharpen::AsyncRwLock::ReadUnlock() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    assert(this->state_ == sharpen::RwLockState::SharedReading);
    this->readers_ -= 1;
    if (this->readers_ != 0)
    {
        return;
    }
    if (!this->writeWaiters_.empty())
    {
        sharpen::AsyncRwLock::MyFuturePtr futurePtr = this->writeWaiters_.back();
        this->writeWaiters_.pop_back();
        this->state_ = sharpen::RwLockState::UniquedWriting;
        lock.unlock();
        futurePtr->Complete(sharpen::RwLockState::SharedReading);
        return;
    }
    this->state_ = sharpen::RwLockState::Free;
}

void sharpen::AsyncRwLock::Unlock() noexcept
{
    if (this->state_ == sharpen::RwLockState::UniquedWriting)
    {
        this->WriteUnlock();
    }
    else if (this->state_ == sharpen::RwLockState::SharedReading)
    {
        this->ReadUnlock();
    }
}

sharpen::RwLockState sharpen::AsyncRwLock::UpgradeFromRead()
{
    MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        assert(this->state_ == sharpen::RwLockState::SharedReading);
        // if only we are reading
        // change lock status
        this->readers_ -= 1;
        if (this->readers_ == 0)
        {
            this->state_ = sharpen::RwLockState::UniquedWriting;
            return sharpen::RwLockState::SharedReading;
        }
        this->writeWaiters_.push_back(&future);
    }
    return future.Await();
}

sharpen::RwLockState sharpen::AsyncRwLock::DowngradeFromWrite()
{
    sharpen::AsyncRwLock::Waiters waiters;
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        assert(this->state_ == sharpen::RwLockState::UniquedWriting);
        this->readers_ = 1;
        std::swap(waiters, this->readWaiters_);
        this->readers_ += static_cast<std::uint32_t>(waiters.size());
        this->state_ = sharpen::RwLockState::SharedReading;
    }
    for (auto begin = waiters.begin(), end = waiters.end(); begin != end; ++begin)
    {
        (*begin)->Complete(sharpen::RwLockState::UniquedWriting);
    }
    return sharpen::RwLockState::UniquedWriting;
}