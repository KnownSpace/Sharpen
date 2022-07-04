#include <sharpen/WorkerGroup.hpp>

#include <sharpen/EventEngine.hpp>

sharpen::WorkerGroup::WorkerGroup(sharpen::EventEngine &engine)
    :WorkerGroup(engine,engine.GetParallelCount())
{}

sharpen::WorkerGroup::WorkerGroup(sharpen::EventEngine &engine,std::size_t workerCount)
    :token_(true)
    ,queue_()
    ,workers_(workerCount)
{
    assert(workerCount != 0);
    for(std::size_t i = 0;i != workerCount;++i)
    {
        engine.Launch(&Self::Entry,this,i);
    }
}

void sharpen::WorkerGroup::Stop() noexcept
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

void sharpen::WorkerGroup::Join() noexcept
{
    for(auto begin = this->workers_.begin(),end = this->workers_.end(); begin != end; ++begin)
    {
        if(begin->IsPending())
        {
            begin->WaitAsync();
        }   
    }
}

void sharpen::WorkerGroup::Entry(std::size_t index)
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