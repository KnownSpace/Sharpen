#include <sharpen/AsyncBarrier.hpp>
#include <cassert>

sharpen::AsyncBarrier::AsyncBarrier(std::uint64_t counter)
    :count_(counter)
    ,waiters_()
    ,currentCount_(0)
{}

std::size_t sharpen::AsyncBarrier::WaitAsync()
{
    sharpen::AsyncBarrier::MyFuture future;
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        if (this->currentCount_ >= this->count_)
        {
            std::size_t currentCount{this->currentCount_};
            this->ResetWithoutLock();
            return currentCount;
        }
        this->waiters_.push_back(&future);
    }
    return future.Await();
}

void sharpen::AsyncBarrier::ResetWithoutLock() noexcept
{
    this->currentCount_ = 0;
}

void sharpen::AsyncBarrier::Reset() noexcept
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    this->ResetWithoutLock();
}

void sharpen::AsyncBarrier::Notify(std::size_t count) noexcept
{
    assert(count);
    MyFuturePtr futurePtr{nullptr};
    std::size_t currentCount{0};
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        this->currentCount_ += count;
        if(this->currentCount_ >= this->count_)
        {
            currentCount = this->currentCount_;
            futurePtr = this->waiters_.back();
            this->waiters_.pop_back();
            this->ResetWithoutLock();
        }
    }
    futurePtr->Complete(currentCount);
}