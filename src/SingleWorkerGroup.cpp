#include <sharpen/SingleWorkerGroup.hpp>

sharpen::SingleWorkerGroup::SingleWorkerGroup(sharpen::EventEngine &engine)
    :token_(true)
    ,queue_()
    ,worker_()
{
    engine.Launch(&Self::Entry,this);
}

sharpen::SingleWorkerGroup::~SingleWorkerGroup() noexcept
{
    this->Stop();
    this->Join();
}

void sharpen::SingleWorkerGroup::Stop() noexcept
{
    bool token{false};
    this->token_.exchange(token);
    if(token)
    {
        this->DoSubmit(std::function<void()>{});
    }
}

void sharpen::SingleWorkerGroup::Join() noexcept
{
    if(this->worker_.IsPending())
    {
        this->worker_.WaitAsync();
    }
}

bool sharpen::SingleWorkerGroup::Running() const noexcept
{
    return this->token_;
}

void sharpen::SingleWorkerGroup::Entry() noexcept
{
    std::function<void()> task;
    while (this->token_)
    {
        task = std::move(this->queue_.Pop());
        if(task)
        {
            sharpen::NonexceptInvoke(task);
        }
    }
    this->worker_.Complete();
}

void sharpen::SingleWorkerGroup::DoSubmit(std::function<void()> task)
{
    assert(this->token_);
    this->queue_.Emplace(std::move(task));
}