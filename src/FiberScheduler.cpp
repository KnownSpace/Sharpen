#include <sharpen/FiberScheduler.hpp>

#include <cassert>

sharpen::BlockingQueue<sharpen::FiberPtr> sharpen::FiberScheduler::fibers_;

thread_local sharpen::FiberPtr sharpen::FiberScheduler::processer_;

thread_local std::unique_ptr<sharpen::FiberScheduler> sharpen::FiberScheduler::scheduler_;


sharpen::FiberScheduler::FiberScheduler()
    :running_(true)
{}

sharpen::FiberScheduler::~FiberScheduler()
{
    this->Stop();
}

bool sharpen::FiberScheduler::IsProcesser() noexcept
{
    return sharpen::FiberScheduler::processer_ != nullptr;
}

void sharpen::FiberScheduler::Stop() noexcept
{
    this->running_ = false;
}

void sharpen::FiberScheduler::AsProcesser()
{
    sharpen::FiberScheduler::processer_ = sharpen::Fiber::GetCurrentFiber();
    while (this->running_)
    {
        Func cb;
        std::swap(cb,this->switchCallback_);
        try
        {
            if (cb)
            {
                cb();
            }
                
        }
        catch(const std::exception& ignore)
        {
            assert(ignore.what() == nullptr);
            (void)ignore;
        }
        this->ProcessOnce(std::chrono::seconds(10));
    }
    sharpen::FiberScheduler::processer_.reset();
}

void sharpen::FiberScheduler::Schedule(sharpen::FiberPtr &&fiber)
{
    sharpen::FiberScheduler::fibers_.Push(std::move(fiber));
}

sharpen::FiberScheduler &sharpen::FiberScheduler::GetScheduler()
{
    if (!sharpen::FiberScheduler::scheduler_)
    {
        auto *p = new sharpen::FiberScheduler();
        if (!p)
        {
            throw std::bad_alloc();
        }
        sharpen::FiberScheduler::scheduler_.reset(p);
    }
    return *sharpen::FiberScheduler::scheduler_;
}

void sharpen::FiberScheduler::SwitchToProcesser()
{
    if (this->IsProcesser())
    {
        sharpen::FiberScheduler::processer_->Switch();
    }
}

void sharpen::FiberScheduler::SwitchToProcesser(Func callback)
{
    if (this->IsProcesser())
    {
        this->switchCallback_ = callback;
        sharpen::FiberScheduler::processer_->Switch();
    }
}