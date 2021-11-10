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
    using FnPtr = void(*)(sharpen::Future<void>*);
    this->cb_ = std::bind(static_cast<FnPtr>(&sharpen::LinuxTimer::CompleteFuture),&future);
    itimerspec time;
    std::memset(&(time.it_interval),0,sizeof(time.it_interval));
    time.it_value.tv_sec = waitMs / 1000;
    time.it_value.tv_nsec = (waitMs % 1000)*1000*1000;
    int r = ::timerfd_settime(this->handle_,0,&time,nullptr);
    if(r == -1)
    {
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

void sharpen::LinuxTimer::CompleteFuture(sharpen::Future<void> *future)
{
    assert(future);
    future->Complete();
}

void sharpen::LinuxTimer::Cancel()
{
    assert(this->handle_ != -1);
    itimerspec time;
    std::memset(&(time.it_interval),0,sizeof(time.it_interval));
    std::memset(&(time.it_value),0,sizeof(time.it_value));
    ::timerfd_settime(this->handle_,0,&time,nullptr);
    Callback cb;
    std::swap(cb,this->cb_);
    if(cb)
    {
        cb();
    }
}

#endif