#include <sharpen/EventLoopThread.hpp>

#include <cassert>

sharpen::EventLoopThread::EventLoopThread(sharpen::SelectorPtr selector)
    : loop_(selector)
    , thread_()
{
    this->thread_ = std::move(std::thread(std::bind(&sharpen::EventLoopThread::Entry, this)));
}

sharpen::EventLoopThread::~EventLoopThread() noexcept
{
    this->Stop();
    this->Join();
}

void sharpen::EventLoopThread::Join()
{
    assert(std::this_thread::get_id() != this->thread_.get_id());
    if (this->thread_.joinable())
    {
        this->thread_.join();
    }
}

void sharpen::EventLoopThread::Detach()
{
    if (this->thread_.joinable())
    {
        this->thread_.detach();
    }
}

void sharpen::EventLoopThread::Stop() noexcept
{
    this->loop_.Stop();
}

void sharpen::EventLoopThread::Entry() noexcept
{
    this->loop_.Run();
}

sharpen::EventLoop *sharpen::EventLoopThread::GetLoop() noexcept
{
    return &this->loop_;
}