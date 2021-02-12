#include <sharpen/WorkerPool.hpp>

#include <cassert>

sharpen::WorkerPool::WorkerPool(sharpen::Uint32 count)
    :running_(false)
    ,count_(count)
    ,stopFlag_(count)
    ,threadFlag_(0)
{
    assert(count > 0);
}

sharpen::WorkerPool::~WorkerPool() noexcept
{
    this->Stop();
}

void sharpen::WorkerPool::Entry() noexcept
{
    try
    {
        std::unique_lock<sharpen::AsyncSemaphore> lock(this->threadFlag_);
        this->stopFlag_.Notice();
    }
    catch(const std::exception& ignore)
    {
        assert(ignore.what() != nullptr);
    }
}

sharpen::Size sharpen::WorkerPool::GetWorkerCount() const noexcept
{
    return this->count_;
}

void sharpen::WorkerPool::Start()
{
    assert(this->running_ != true);
    this->running_ = true;
    for (sharpen::Size i = 0; i < this->count_; ++i)
    {
        std::thread t(std::bind(&sharpen::WorkerPool::Entry,this));
        t.detach();
    }
}

void sharpen::WorkerPool::Stop() noexcept
{
    if (this->running_)
    {
        this->running_ = false;
        this->threadFlag_.Unlock(this->count_);
        this->stopFlag_.WaitAsync();
    }
}