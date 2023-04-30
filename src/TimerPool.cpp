#include <sharpen/TimerPool.hpp>

sharpen::TimerPool::TimerPool(sharpen::IEventLoopGroup &loopGroup,
                              TimerMaker maker,
                              std::size_t reserveCount)
    : lock_()
    , timers_()
    , maker_(maker)
    , loopGroup_(&loopGroup)
{
    if (!reserveCount)
    {
        this->timers_.reserve(reservedSize_);
    }
    this->Reserve(reserveCount);
}

void sharpen::TimerPool::Reserve(std::size_t size)
{
    if (size)
    {
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            std::size_t realSize{this->timers_.size() + size};
            this->timers_.reserve(realSize);
            for (std::size_t i = 0; i != size; ++i)
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
        if (!this->timers_.empty())
        {
            timer = std::move(this->timers_.back());
            this->timers_.pop_back();
        }
    }
    if (!timer)
    {
        timer = this->MakeTimer();
    }
    return timer;
}

void sharpen::TimerPool::ReturnTimer(sharpen::TimerPtr &&timer) noexcept
{
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        try
        {
            this->timers_.emplace_back(std::move(timer));
        }
        catch (const std::bad_alloc &ignore)
        {
            // drop timer
            (void)timer;
            (void)ignore;
        }
    }
}