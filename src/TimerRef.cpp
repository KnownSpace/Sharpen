#include <sharpen/TimerRef.hpp>

sharpen::TimerUniquedRef::TimerUniquedRef(sharpen::ITimerPool &pool)
    :Self(pool.GetTimer(),pool)
{}

sharpen::TimerUniquedRef::TimerUniquedRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept
    :pool_(&pool)
    ,timer_(std::move(timer))
{}

sharpen::TimerUniquedRef::TimerUniquedRef(sharpen::TimerUniquedRef &&other) noexcept
    :pool_(other.pool_)
    ,timer_(std::move(other.timer_))
{
    other.pool_ = nullptr;
}

sharpen::TimerUniquedRef &sharpen::TimerUniquedRef::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->pool_ = other.pool_;
        this->timer_ = std::move(other.timer_);
        other.pool_ = nullptr;
    }
    return *this;
}

sharpen::TimerUniquedRef::~TimerUniquedRef() noexcept
{
    if(this->timer_)
    {
        assert(this->pool_);
        this->pool_->ReturnTimer(std::move(this->timer_));
    }
}

sharpen::TimerRef::TimerRef(sharpen::ITimerPool &pool)
    :Self(pool.GetTimer(),pool)
{}

sharpen::TimerRef::TimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept
    :realTimer_(std::make_shared<sharpen::TimerUniquedRef>(std::move(timer),pool))
{}

sharpen::TimerRef &sharpen::TimerRef::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->realTimer_ = std::move(other.realTimer_);
    }
    return *this;
}