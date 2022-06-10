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

sharpen::EventEngine::EventEngine(std::size_t workerCount)
    :workers_()
    ,pos_(0)
    ,mainLoop_(nullptr)
{
    assert(workerCount != 0);
    this->mainLoop_.reset(new sharpen::EventLoop(sharpen::MakeDefaultSelector()));
    this->loops_.push_back(this->mainLoop_.get());
    for (size_t i = 0,count = workerCount - 1; i < count; i++)
    {
        //one selector per thread
        std::unique_ptr<sharpen::EventLoopThread> thread(new sharpen::EventLoopThread(sharpen::MakeDefaultSelector()));
        this->loops_.push_back(thread->GetLoop());
        this->workers_.push_back(std::move(thread));
    }
}

sharpen::EventEngine::~EventEngine() noexcept
{
    this->Stop();
}

sharpen::EventLoop *sharpen::EventEngine::RoundRobinLoop() noexcept
{
    std::size_t pos = this->pos_++;
    return this->loops_[pos % this->loops_.size()];
}

void sharpen::EventEngine::Stop() noexcept
{
    for (auto begin = this->workers_.begin(),end = this->workers_.end();begin != end;++begin)
    {
        (*begin)->Stop();
    }
    this->mainLoop_->Stop();
}

void sharpen::EventEngine::CallSwitchCallback()
{
    SwitchCallback cb;
    std::swap(cb,sharpen::EventEngine::switchCb_);
    if (cb)
    {
        cb();
    }
}

void sharpen::EventEngine::Schedule(sharpen::FiberPtr &&fiber)
{
    using FnPtr = void(*)(sharpen::FiberPtr);
    auto &&fn = std::bind(static_cast<FnPtr>(&sharpen::EventEngine::ProcessFiber),std::move(fiber));
    if (this->IsProcesser() && sharpen::EventLoop::GetLocalFiber() == sharpen::Fiber::GetCurrentFiber())
    {
        fn();
        return;
    }
    //find a waiting loop
    for (auto begin = this->loops_.begin(),end = this->loops_.end(); begin != end; begin++)
    {
        sharpen::EventLoop *loop = *begin;
        if (loop->IsWaiting())
        {
            loop->RunInLoopSoon(std::move(fn));
            return;
        }
    }
    //could not found a waiting loop
    //round robin schedule
    this->RoundRobinLoop()->RunInLoopSoon(std::move(fn));
}

void sharpen::EventEngine::ProcessFiber(sharpen::FiberPtr fiber)
{
    try
    {
        sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
        fiber->Switch(current);
    }
    catch(const std::bad_alloc &fault)
    {
        (void)fault;
        std::terminate();
    }
    catch(std::system_error &error)
    {
        if(sharpen::IsFatalError(error.code().value()))
        {
            std::terminate();
        }
    }
    catch(const std::exception &ignore)
    {
        assert(ignore.what() == nullptr && "an exception occured in event loop");
        (void)ignore;
    }
    sharpen::EventEngine::CallSwitchCallback();
}

sharpen::EventEngine &sharpen::EventEngine::SetupEngine(std::size_t workerCount)
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

void sharpen::EventEngine::ProcessStartup(std::function<void()> fn)
{
    if(fn)
    {
        try
        {
            fn();
        }
        catch(const std::bad_alloc &fault)
        {
            (void)fault;
            std::terminate();
        }
        catch(std::system_error &error)
        {
            if(sharpen::IsFatalError(error.code().value()))
            {
                std::terminate();
            }
        }
        catch(const std::exception &ignore)
        {
            assert(ignore.what() == nullptr && "an exception occured in event engine");
            (void)ignore;
        }
    }
    this->Stop();
}

void sharpen::EventEngine::ProcessStartupWithCode(std::function<int()> fn,int *code)
{
    assert(code != nullptr);
    if(fn)
    {
        try
        {
            *code = fn();
        }
        catch(const std::bad_alloc &fault)
        {
            (void)fault;
            std::terminate();
        }
        catch(std::system_error &error)
        {
            if(sharpen::IsFatalError(error.code().value()))
            {
                std::terminate();
            }
        }
        catch(const std::exception &ignore)
        {
            assert(ignore.what() == nullptr && "an exception occured in event engine");
            (void)ignore;
        }
    }
    this->Stop();
}