#include <sharpen/FiberProcesserPool.hpp>

sharpen::FiberProcesserPool::FiberProcesserPool()
    :FiberProcesserPool(std::thread::hardware_concurrency())
{}

sharpen::FiberProcesserPool::FiberProcesserPool(sharpen::Size size)
{
    for (sharpen::Size i = 0; i < size; i++)
    {
        this->processers_.push_back(std::move(sharpen::FiberProcesser()));
    }
}

sharpen::FiberProcesserPool::~FiberProcesserPool() noexcept
{
    this->Stop();
}

void sharpen::FiberProcesserPool::Stop() noexcept
{
    if (this->running_)
    {
        for (auto begin = this->processers_.begin(),end = this->processers_.end(); begin != end; begin++)
        {
            begin->Stop();
        }
        this->processers_.clear();
    }
}

sharpen::Size sharpen::FiberProcesserPool::GetSize() const noexcept
{
    return this->processers_.size();
}