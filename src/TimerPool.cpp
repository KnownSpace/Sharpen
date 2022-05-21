#include <sharpen/TimerPool.hpp>

sharpen::TimerPool::TimerPool(sharpen::EventEngine &engine,TimerMaker maker,sharpen::Size reserveCount)
    :lock_()
    ,timers_()
    ,maker_(maker)
    ,engine_(&engine)
{
    this->Reserve(reserveCount);
}

void sharpen::TimerPool::Reserve(sharpen::Size size)
{
    if(size)
    {
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            this->timers_.reserve(size);
            for (sharpen::Size i = 0; i != size; ++i)
            {
                this->timers_.emplace_back(this->MakeTimer());   
            }
        }
    }
}

sharpen::TimerPtr sharpen::TimerPool::GetTimer()
{
    sharpen::TimerPtr timer{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if(!this->timers_.empty())
        {
            timer = std::move(this->timers_.back());
            this->timers_.pop_back();
        }
    }
    if(!timer)
    {
        timer = this->MakeTimer();
    }
    return timer;
}

void sharpen::TimerPool::PutTimer(sharpen::TimerPtr &&timer)
{
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        this->timers_.emplace_back(std::move(timer));
    }
}