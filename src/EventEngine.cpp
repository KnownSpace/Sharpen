#include <sharpen/EventEngine.hpp>
#include <sharpen/ISelector.hpp>
#include <sharpen/SystemMacro.hpp>
#include <cassert>

sharpen::EventEngine::EventEngine()
    :EventEngine(std::thread::hardware_concurrency())
{}

sharpen::EventEngine::EventEngine(sharpen::Size workerCount)
    :workers_()
    ,pos_(0)
{
    assert(workerCount != 0);
    for (size_t i = 0; i < workerCount; i++)
    {
#ifdef SHARPEN_IS_WIN
        //global selector
        sharpen::SelectorPtr selector = sharpen::MakeDefaultSelector();
        auto tasks = std::make_shared<std::vector<std::function<void()>>>();
        auto lock = std::make_shared<sharpen::SpinLock>();
        std::unique_ptr<sharpen::EventLoopThread> thread(new sharpen::EventLoopThread(selector,tasks,lock));
        this->workers_.push_back(std::move(thread));
#else
        //one selector per thread
        std::unique_ptr<sharpen::EventLoopThread> thread(new sharpen::EventLoopThread(sharpen::MakeDefaultSelector()));
        this->workers_.push_back(std::move(thread));
#endif
    }
}

sharpen::EventEngine::~EventEngine() noexcept
{
    this->Stop();
}

sharpen::EventLoop *sharpen::EventEngine::RoundRobinLoop() noexcept
{
    sharpen::Size pos = this->pos_;
    this->pos_++;
    pos %= this->workers_.size();
    return this->workers_[pos]->GetLoop();
}

void sharpen::EventEngine::Stop() noexcept
{
    for (auto begin = this->workers_.begin(),end = this->workers_.end();begin != end;++begin)
    {
        (*begin)->Stop();
    }
    this->workers_.clear();
}