#include <sharpen/TimerLoop.hpp>

#include <sharpen/EventEngine.hpp>

sharpen::TimerLoop::TimerLoop(sharpen::EventEngine &engine,sharpen::TimerPtr timer,Handler handler,std::function<WaitTime()> waittimeGenerator)
    :timer_(std::move(timer))
    ,handler_(std::move(handler))
    ,waitTimeGenerator_(std::move(waittimeGenerator))
    ,future_()
    ,token_(false)
    ,engine_(&engine)
{
    assert(this->timer_);
    assert(this->waitTimeGenerator_);
    assert(this->handler_);
}

sharpen::TimerLoop::TimerLoop(Self &&other) noexcept
    :timer_(std::move(other.timer_))
    ,handler_(std::move(other.handler_))
    ,waitTimeGenerator_(std::move(other.waitTimeGenerator_))
    ,future_(std::move(other.future_))
    ,token_(other.token_.load())
    ,engine_(other.engine_)
{}

void sharpen::TimerLoop::Entry()
{
    sharpen::AwaitableFuture<bool> future;
    while (this->token_)
    {
        future.Reset();
        WaitTime time{this->waitTimeGenerator_()};
        this->timer_->WaitAsync(future,time);
        if(future.Await())
        {
            try
            {
                //should be noexcept
                if(this->handler_() == Self::LoopStatus::Terminate)
                {
                    this->future_.Complete();
                    this->token_ = false;
                    return;
                }
            }
            catch(const std::exception& ignore)
            {
                static_cast<void>(ignore);
                assert(ignore.what() == nullptr);
            }
        }
    }
    this->future_.Complete();
}

void sharpen::TimerLoop::Cancel()
{
    if(this->token_)
    {
        this->timer_->Cancel();
    }
}

void sharpen::TimerLoop::Terminate()
{
    bool token{this->token_.exchange(false)};
    if(token && this->future_.IsPending())
    {
        this->future_.WaitAsync();
        this->future_.Reset();
    }
}

void sharpen::TimerLoop::Start()
{
    bool token{this->token_.exchange(true)};
    if(!token)
    {
        this->future_.Reset();
        try
        {
            this->engine_->Launch(std::bind(&Self::Entry,this));
        }
        catch(const std::exception&)
        {
            this->token_ = false;
            throw;
        }
    }
}

sharpen::TimerLoop &sharpen::TimerLoop::operator=(sharpen::TimerLoop &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->timer_ = std::move(other.timer_);
        this->handler_ = std::move(other.handler_);
        this->waitTimeGenerator_ = std::move(other.waitTimeGenerator_);
        this->future_ = std::move(other.future_);
        this->token_ = other.token_.load();
        this->engine_ = other.engine_;
    }
    return *this;
}