#include <sharpen/EventLoop.hpp>

#include <cassert>

thread_local sharpen::EventLoop *sharpen::EventLoop::localLoop_(nullptr);

thread_local sharpen::FiberPtr sharpen::EventLoop::localFiber_;

sharpen::EventLoop::EventLoop(SelectorPtr selector)
    :selector_(selector)
    ,tasks_(std::make_shared<TaskVector>())
    ,exectingTask_(false)
    ,lock_(std::make_shared<Lock>())
    ,running_(false)
{
    assert(selector != nullptr);
}

sharpen::EventLoop::~EventLoop() noexcept
{
    this->Stop();
}

void sharpen::EventLoop::Bind(WeakChannelPtr channel)
{
    this->selector_->Resister(channel);
}

void sharpen::EventLoop::RunInLoop(Task task)
{
    if (this->GetLocalLoop() == this)
    {
        try
        {
            task();
        }
        catch(const std::exception& ignore)
        {
            assert(ignore.what() == nullptr);
            (void)ignore;
        }
        return;
    }
    this->RunInLoopSoon(std::move(task));
}

void sharpen::EventLoop::RunInLoopSoon(Task task)
{
    bool execting(true);
    {
        std::unique_lock<Lock> lock(*this->lock_);
        this->tasks_->push_back(std::move(task));
        std::swap(execting,this->exectingTask_);
    }
    if (!execting)
    {
        this->selector_->Notify();
    }
}

void sharpen::EventLoop::ExecuteTask()
{
    TaskVector tasks;
    {
        std::unique_lock<Lock> lock(*this->lock_);
        this->exectingTask_ = false;
        if (this->tasks_->empty())
        {
            return;
        }
        std::swap(*this->tasks_,tasks);
    }
    for (auto begin = tasks.begin(),end = tasks.end();begin != end;++begin)
    {
        try
        {
            if (*begin)
            {
                (*begin)();
            }
        }
        catch(const std::exception& ignore)
        {
            assert(ignore.what() == nullptr);
            (void)ignore;
        }
    }
}

void sharpen::EventLoop::Run()
{
    if (sharpen::EventLoop::IsInLoop())
    {
        throw std::logic_error("now is in event loop");
    }
    sharpen::EventLoop::localLoop_ = this;
    sharpen::EventLoop::localFiber_ = sharpen::Fiber::GetCurrentFiber();
    EventVector events;
    events.reserve(32);
    this->running_ = true;
    while (this->running_)
    {
        this->wait_ = true;
        //select events
        this->selector_->Select(events);
        this->wait_ = false;
        for (auto begin = events.begin(),end = events.end();begin != end;++begin)
        {
            sharpen::ChannelPtr channel = (*begin)->GetChannel();
            if (channel)
            {
                channel->OnEvent(*begin);
            }
        }
        events.clear();
        //execute tasks
        this->ExecuteTask();
    }
    sharpen::EventLoop::localLoop_ = nullptr;
    sharpen::EventLoop::localFiber_.reset();
}

bool sharpen::EventLoop::IsWaiting() const noexcept
{
    return this->wait_;
}

sharpen::FiberPtr sharpen::EventLoop::GetLocalFiber() noexcept
{
    return sharpen::EventLoop::localFiber_;
}

void sharpen::EventLoop::Stop() noexcept
{
    if (this->running_)
    {
        this->running_ = false;
        this->selector_->Notify();
    }
}

sharpen::EventLoop *sharpen::EventLoop::GetLocalLoop() noexcept
{
    return sharpen::EventLoop::localLoop_;
}

bool sharpen::EventLoop::IsInLoop() noexcept
{
    return sharpen::EventLoop::GetLocalLoop() != nullptr;
}