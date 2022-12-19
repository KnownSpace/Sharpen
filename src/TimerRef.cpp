#include <sharpen/TimerRef.hpp>

sharpen::UniquedTimerRef::UniquedTimerRef(sharpen::ITimerPool &pool)
    :Self(pool.GetTimer(),pool)
{}

sharpen::UniquedTimerRef::UniquedTimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept
    :pool_(&pool)
    ,timer_(std::move(timer))
{}

sharpen::UniquedTimerRef::UniquedTimerRef(sharpen::UniquedTimerRef &&other) noexcept
    :pool_(other.pool_)
    ,timer_(std::move(other.timer_))
{
    other.pool_ = nullptr;
}

sharpen::UniquedTimerRef &sharpen::UniquedTimerRef::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->pool_ = other.pool_;
        this->timer_ = std::move(other.timer_);
        other.pool_ = nullptr;
    }
    return *this;
}

sharpen::UniquedTimerRef::~UniquedTimerRef() noexcept
{
    if(this->timer_)
    {
        assert(this->pool_);
        this->pool_->ReturnTimer(std::move(this->timer_));
    }
}

sharpen::SharedTimerRef::SharedTimerRef(sharpen::ITimerPool &pool)
    :Self(pool.GetTimer(),pool)
{}

sharpen::SharedTimerRef::SharedTimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept
    :realTimer_(std::make_shared<sharpen::UniquedTimerRef>(std::move(timer),pool))
{}

sharpen::SharedTimerRef &sharpen::SharedTimerRef::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->realTimer_ = std::move(other.realTimer_);
    }
    return *this;
}