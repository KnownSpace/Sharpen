#include <sharpen/AsyncLeaseLock.hpp>

#include <sharpen/Fiber.hpp>
#include <sharpen/TimerOps.hpp>
#include <cassert>

sharpen::AsyncLeaseLock::AsyncLeaseLock(std::uint32_t waitMs)
    : leaseDuration_(std::chrono::milliseconds{waitMs})
    , lock_()
    , queueLock_()
    , ownerId_(0)
    , grantTime_()
    , waiter_(nullptr) {
    assert(this->leaseDuration_.count());
    if (!this->leaseDuration_.count()) {
        throw std::invalid_argument{"lease too short"};
    }
}

void sharpen::AsyncLeaseLock::LockAsync(sharpen::TimerPtr timer) {
    assert(timer != nullptr);
    std::uint64_t id{sharpen::GetCurrentFiberId()};
    sharpen::AwaitableFuture<bool> future;
    {
        std::unique_lock<sharpen::AsyncMutex> queueLock{this->queueLock_};
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            TimePoint now{std::chrono::steady_clock::now()};
            if (!this->ownerId_) {
                this->grantTime_ = now;
                this->ownerId_ = id;
                return;
            }
            TimePoint timeout{this->grantTime_ + this->leaseDuration_};
            while (this->ownerId_ && now < timeout) {
                std::chrono::milliseconds wait{
                    std::chrono::duration_cast<std::chrono::milliseconds>(timeout - now)};
                if (wait.count()) {
                    future.Reset();
                    timer->WaitAsync(future, wait);
                    this->waiter_ = timer.get();
                    lock.unlock();
                    bool ok{future.Await()};
                    (void)ok;
                    lock.lock();
                }
                now = std::chrono::steady_clock::now();
            }
            this->grantTime_ = now;
            this->ownerId_ = id;
            this->waiter_ = nullptr;
        }
    }
}

void sharpen::AsyncLeaseLock::LockAsync() {
    sharpen::UniquedTimerRef timer{sharpen::GetUniquedTimerRef()};
    this->LockAsync(timer.Timer());
}

void sharpen::AsyncLeaseLock::Unlock() noexcept {
    std::uint64_t id{sharpen::GetCurrentFiberId()};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if (this->ownerId_ == id) {
            this->ownerId_ = 0;
            if (this->waiter_ != nullptr) {
                this->waiter_->Cancel();
            }
        }
    }
}