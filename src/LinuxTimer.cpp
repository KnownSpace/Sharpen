#include <sharpen/LinuxTimer.hpp>
#ifdef SHARPEN_HAS_TIMERFD

#include <cstring>
#include <sys/timerfd.h>
#include <cassert>

#include <sharpen/SystemError.hpp>

sharpen::LinuxTimer::LinuxTimer()
    :MyTimerBase()
    ,Mybase()
    ,cb_()
{
    this->handle_ = ::timerfd_create(CLOCK_REALTIME,TFD_NONBLOCK | TFD_CLOEXEC);
    if(this->handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::LinuxTimer::WaitAsync(sharpen::Future<void> &future,sharpen::Uint64 waitMs)
{
    assert(this->handle_ != -1);
    using FnPtr = void(*)(sharpen::LinuxTimer::WaitStruct*);
    sharpen::LinuxTimer::WaitStruct *wait(new sharpen::LinuxTimer::WaitStruct());
    if(!wait)
    {
        throw std::bad_alloc();
    }
    wait->localTick_ = this->tick_.load();
    wait->timer_ = this;
    this->future_ = &future;
    this->cb_ = std::bind(reinterpret_cast<FnPtr>(&sharpen::LinuxTimer::CompleteFuture),wait);
    itimerspec time;
    std::memset(&(time.it_interval),0,sizeof(time.it_interval));
    time.it_value.tv_sec = waitMs / 1000;
    time.it_value.tv_nsec = (waitMs % 1000)*1000*1000;
    int r = ::timerfd_settime(this->handle_,0,&time,nullptr);
    if(r == -1)
    {
        delete wait;
        sharpen::ThrowLastError();
    }
}

void sharpen::LinuxTimer::OnEvent(sharpen::IoEvent *event)
{
    if(!event->IsReadEvent())
    {
        return;
    }
    Callback cb;
    std::swap(cb,this->cb_);
    if(cb)
    {
        cb();
    }
}

void sharpen::LinuxTimer::CompleteFuture(sharpen::LinuxTimer::WaitStruct *arg)
{
    assert(arg != nullptr);
    std::unique_ptr<sharpen::LinuxTimer::WaitStruct> wait(arg);
    if(wait->localTick_ == wait->timer_->tick_.load())
    {
        sharpen::Future<void> *future;
        std::swap(wait->timer_->future_,future);
        if(future)
        {
            future->Complete();
        }
    }
}

void sharpen::LinuxTimer::Cancel()
{
    this->tick_.fetch_add(1);
    sharpen::Future<void> *future{nullptr};
    std::swap(future,this->future_);
    if(future)
    {
        future->Complete();
    }
}

#endif