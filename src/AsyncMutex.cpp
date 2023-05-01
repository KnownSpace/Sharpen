#include <sharpen/AsyncMutex.hpp>

sharpen::AsyncMutex::AsyncMutex()
    : locked_(false)
    , waiters_()
    , lock_() {
}

void sharpen::AsyncMutex::LockAsync() {
    sharpen::AsyncMutex::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (!this->locked_) {
            this->locked_ = true;
            return;
        }
        this->waiters_.push_back(&future);
    }
    future.Await();
}

bool sharpen::AsyncMutex::TryLock() noexcept {
    {
        if (!this->lock_.TryLock()) {
            return false;
        }
        std::unique_lock<sharpen::SpinLock> lock{this->lock_, std::adopt_lock};
        bool locked{true};
        std::swap(locked, this->locked_);
        return !locked;
    }
}

void sharpen::AsyncMutex::Unlock() noexcept {
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->waiters_.empty()) {
        this->locked_ = false;
        return;
    }
    sharpen::AsyncMutex::MyFuturePtr futurePtr = this->waiters_.back();
    this->waiters_.pop_back();
    lock.unlock();
    futurePtr->Complete();
}
