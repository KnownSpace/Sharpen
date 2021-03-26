#include <cassert>

#include <sharpen/Awaiter.hpp>

sharpen::Awaiter::Awaiter()
    :fiber_()
{}

sharpen::Awaiter::Awaiter(sharpen::Awaiter::Self &&other) noexcept
    :fiber_(std::move(other.fiber_))
{}

sharpen::Awaiter::Self &sharpen::Awaiter::operator=(sharpen::Awaiter::Self &&other) noexcept
{
    this->fiber_ = std::move(other.fiber_);
    return *this;
}

void sharpen::Awaiter::Notify()
{
    if (this->fiber_)
    {
        sharpen::FiberScheduler::GetScheduler().Schedule(std::move(this->fiber_));
    }
}

void sharpen::Awaiter::Wait(sharpen::FiberPtr fiber)
{
    {
        assert(this->fiber_ == nullptr);
        this->fiber_ = std::move(fiber);
    }
}