#include <sharpen/Awaiter.hpp>
#include <cassert>

sharpen::Awaiter::Awaiter()
    :waiter_()
{}

sharpen::Awaiter::Awaiter(sharpen::Awaiter::Self &&other) noexcept
    :waiter_(std::move(other.waiter_))
{}

sharpen::Awaiter::Self &sharpen::Awaiter::operator=(sharpen::Awaiter::Self &&other) noexcept
{
    this->waiter_ = std::move(other.waiter_);
    return *this;
}

void sharpen::Awaiter::Notify()
{
    {
        if (this->waiter_)
        {
            sharpen::CentralEngine.PushContext(std::move(this->waiter_));
        }
    }
}

void sharpen::Awaiter::Wait(sharpen::ExecuteContextPtr context)
{
    {
        assert(this->waiter_ == nullptr);
        this->waiter_ = std::move(context);
    }
}