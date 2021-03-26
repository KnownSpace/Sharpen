#include <sharpen/FiberProcesser.hpp>

sharpen::FiberProcesser::FiberProcesser()
    :thread_()
    ,scheduler_(nullptr)
{
    thread_ = std::move(std::thread(std::bind(&sharpen::FiberProcesser::ProcesserEntry,this)));
    while (!this->scheduler_)
    {
        std::this_thread::yield();
    }
}

sharpen::FiberProcesser::FiberProcesser(sharpen::FiberProcesser &&other) noexcept
    :thread_(std::move(other.thread_))
    ,scheduler_(other.scheduler_)
{
    other.scheduler_ = nullptr;
}

sharpen::FiberProcesser::~FiberProcesser() noexcept
{
    this->Stop();
    this->Join();
}

sharpen::FiberProcesser &sharpen::FiberProcesser::operator=(sharpen::FiberProcesser &&other) noexcept
{
    this->thread_ = std::move(other.thread_);
    this->scheduler_ = other.scheduler_;
    other.scheduler_ = nullptr;
    return *this;
}

void sharpen::FiberProcesser::ProcesserEntry()
{
    sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
    this->scheduler_ = &scheduler;
    scheduler.AsProcesser();
}

void sharpen::FiberProcesser::Stop() noexcept
{
    if (this->scheduler_)
    {
        this->scheduler_->Stop();
    }
}

void sharpen::FiberProcesser::Join()
{
    if (this->thread_.joinable())
    {
        this->thread_.join();
    }
}

void sharpen::FiberProcesser::Detach()
{
    if (this->thread_.joinable())
    {
        this->thread_.detach();
    }
}