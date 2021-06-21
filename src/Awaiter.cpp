#include <cassert>
#include <mutex>
#include <sharpen/Awaiter.hpp>

sharpen::Awaiter::Awaiter()
    :fiber_()
    ,lock_()
{}

void sharpen::Awaiter::Notify()
{
    sharpen::FiberPtr fiber;
    {
        std::unique_lock<Lock> lock(this->lock_);
        std::swap(fiber,this->fiber_);
    }
    if (fiber)
    {
        sharpen::FiberScheduler::GetScheduler().Schedule(std::move(fiber));
    }
}

void sharpen::Awaiter::Wait(sharpen::FiberPtr fiber)
{
    {
        std::unique_lock<Lock> lock(this->lock_);
        assert(this->fiber_ == nullptr);
        this->fiber_ = std::move(fiber);
    }
}