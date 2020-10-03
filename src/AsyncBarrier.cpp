#include <sharpen/AsyncBarrier.hpp>

sharpen::AsyncBarrier::AsyncBarrier(sharpen::Uint32 counter)
    :counter_(counter)
    ,waiters_()
    ,beginCounter_(counter)
{}

void sharpen::AsyncBarrier::Wait(sharpen::AsyncBarrier::Function &&callback)
{
    std::unique_lock<sharpen::SpinLock> lock(this->lock_);
    if (this->counter_ == 0)
    {
        lock.unlock();
        return;
    }
    this->counter_ -= 1;
}