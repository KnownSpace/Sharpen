#include <sharpen/FixedWorkerGroup.hpp>

sharpen::FixedWorkerGroup::FixedWorkerGroup()
    :FixedWorkerGroup(sharpen::GetLocalScheduler())
{}

sharpen::FixedWorkerGroup::FixedWorkerGroup(sharpen::IFiberScheduler &scheduler)
    :FixedWorkerGroup(scheduler,scheduler.GetParallelCount())
{}

sharpen::FixedWorkerGroup::FixedWorkerGroup(sharpen::IFiberScheduler &scheduler,std::size_t workerCount)
    :token_(true)
    ,queue_()
    ,workers_(workerCount)
{
    assert(workerCount != 0);
    for(std::size_t i = 0;i != workerCount;++i)
    {
        scheduler.Launch(&Self::Entry,this,i);
    }
}

void sharpen::FixedWorkerGroup::Stop() noexcept
{
    bool token{this->token_.exchange(false)};
    if(token)
    {
        for (std::size_t i = 0; i != this->workers_.size(); ++i)
        {
            this->queue_.Emplace();
        }
    }
}

void sharpen::FixedWorkerGroup::Join() noexcept
{
    for(auto begin = this->workers_.begin(),end = this->workers_.end(); begin != end; ++begin)
    {
        if(begin->IsPending())
        {
            begin->WaitAsync();
        }   
    }
}

void sharpen::FixedWorkerGroup::Entry(std::size_t index) noexcept
{
    sharpen::AwaitableFuture<void> *future{&this->workers_[index]};
    assert(future != nullptr);
    std::function<void()> task;
    while(this->token_.load())
    {
        task = std::move(this->queue_.Pop());
        if(task)
        {
            sharpen::NonexceptInvoke(task);
        }
    }
    future->Complete();
}

void sharpen::FixedWorkerGroup::NviSubmit(std::function<void()> task)
{
    assert(this->token_.load());
    this->queue_.Emplace(std::move(task));
}