#include <sharpen/EventEngine.hpp>
#include <sharpen/ISelector.hpp>
#include <sharpen/SystemMacro.hpp>
#include <cassert>

sharpen::EventEngine::SelfPtr sharpen::EventEngine::engine_;

std::once_flag sharpen::EventEngine::flag_;

thread_local sharpen::EventEngine::SwitchCallback sharpen::EventEngine::switchCb_;

sharpen::EventEngine::EventEngine()
    :EventEngine(std::thread::hardware_concurrency())
{}

sharpen::EventEngine::EventEngine(sharpen::Size workerCount)
    :workers_()
    ,pos_(0)
    ,mainLoop_(nullptr)
{
    assert(workerCount != 0);
    this->mainLoop_.reset(new sharpen::EventLoop(sharpen::MakeDefaultSelector()));
    for (size_t i = 0,count = workerCount - 1; i < count; i++)
    {
        //one selector per thread
        std::unique_ptr<sharpen::EventLoopThread> thread(new sharpen::EventLoopThread(sharpen::MakeDefaultSelector()));
        this->workers_.push_back(std::move(thread));
    }
}

sharpen::EventEngine::~EventEngine() noexcept
{
    this->Stop();
}

sharpen::EventLoop *sharpen::EventEngine::RoundRobinLoop() noexcept
{
    sharpen::Size pos = this->pos_;
    sharpen::Size size = this->workers_.size() + 1;
    this->pos_++;
    pos %= size;
    if (pos)
    {
        return this->workers_[pos - 1]->GetLoop();
    }
    return this->mainLoop_.get();
}

void sharpen::EventEngine::Stop() noexcept
{
    for (auto begin = this->workers_.begin(),end = this->workers_.end();begin != end;++begin)
    {
        (*begin)->Stop();
    }
    this->workers_.clear();
    this->mainLoop_->Stop();
}

void sharpen::EventEngine::Schedule(sharpen::FiberPtr &&fiber)
{
    using FnPtr = void(*)(sharpen::FiberPtr);
    auto &&fn = std::bind(reinterpret_cast<FnPtr>(&sharpen::EventEngine::ProcessFiber),std::move(fiber));
    this->RoundRobinLoop()->RunInLoopSoon(std::move(fn));
}

void sharpen::EventEngine::ProcessFiber(sharpen::FiberPtr fiber)
{
    try
    {
        sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
        fiber->Switch(current);
    }
    catch(const std::exception& ignore)
    {
        assert(ignore.what() == nullptr);
        (void)ignore;
    }
    SwitchCallback cb;
    std::swap(cb,sharpen::EventEngine::switchCb_);
    if (cb)
    {
        cb();
    }
}

sharpen::EventEngine &sharpen::EventEngine::SetupEngine(sharpen::Size workerCount)
{
    std::call_once(sharpen::EventEngine::flag_,[workerCount]()
    {
        sharpen::EventEngine *engine = new sharpen::EventEngine(workerCount);
        sharpen::EventEngine::engine_.reset(engine);
    });
    return *sharpen::EventEngine::engine_;
}

sharpen::EventEngine &sharpen::EventEngine::SetupEngine()
{
    std::call_once(sharpen::EventEngine::flag_,[]()
    {
        sharpen::EventEngine *engine = new sharpen::EventEngine();
        sharpen::EventEngine::engine_.reset(engine);
    });
    return *sharpen::EventEngine::engine_;
}

sharpen::EventEngine &sharpen::EventEngine::GetEngine()
{
    if (!sharpen::EventEngine::engine_)
    {
        throw std::logic_error("event engine isn't set");
    }
    return *sharpen::EventEngine::engine_;
}

sharpen::EventEngine &sharpen::EventEngine::SetupSingleThreadEngine()
{
    return sharpen::EventEngine::SetupEngine(1);
}

bool sharpen::EventEngine::IsProcesser() const
{
    return sharpen::EventLoop::IsInLoop();
}

void sharpen::EventEngine::SwitchToProcesserFiber() noexcept
{
    sharpen::EventLoop::GetLocalFiber()->Switch();
}

void sharpen::EventEngine::SetSwitchCallback(std::function<void()> fn)
{
    sharpen::EventEngine::switchCb_ = std::move(fn);
}

void sharpen::EventEngine::Run()
{
    this->mainLoop_->Run();
}