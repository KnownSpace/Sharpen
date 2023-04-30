#include <sharpen/LinuxTimer.hpp>
#ifdef SHARPEN_HAS_TIMERFD

#include <sharpen/EventLoop.hpp>
#include <sharpen/SystemError.hpp>
#include <sys/timerfd.h>
#include <cassert>
#include <cstring>

sharpen::LinuxTimer::LinuxTimer()
    : MyTimerBase()
    , Mybase()
    , future_(nullptr)
{
    this->handle_ = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if (this->handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::LinuxTimer::WaitAsync(sharpen::Future<bool> &future, std::uint64_t waitMs)
{
    assert(this->handle_ != -1);
    if (waitMs == 0)
    {
        future.Complete(true);
        return;
    }
    this->future_ = &future;
    itimerspec time;
    std::memset(&(time.it_interval), 0, sizeof(time.it_interval));
    time.it_value.tv_sec = waitMs / 1000;
    time.it_value.tv_nsec = (waitMs % 1000) * 1000 * 1000;
    int r = ::timerfd_settime(this->handle_, 0, &time, nullptr);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::LinuxTimer::OnEvent(sharpen::IoEvent *event)
{
    if (!event->IsReadEvent())
    {
        return;
    }
    sharpen::Future<bool> *future{this->future_.exchange(nullptr)};
    if (future)
    {
        future->Complete(true);
    }
}

void sharpen::LinuxTimer::Cancel()
{
    if (!this->future_)
    {
        return;
    }
    assert(this->handle_ != -1);
    itimerspec time;
    std::memset(&(time.it_interval), 0, sizeof(time.it_interval));
    std::memset(&(time.it_value), 0, sizeof(time.it_value));
    ::timerfd_settime(this->handle_, 0, &time, nullptr);
    sharpen::Future<bool> *future{this->future_.exchange(nullptr)};
    if (future)
    {
        future->Complete(false);
    }
}

#endif