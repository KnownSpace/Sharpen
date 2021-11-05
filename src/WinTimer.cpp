#include <sharpen/WinTimer.hpp>

#ifdef SHARPEN_HAS_WAITABLETIMER

#include <Windows.h>
#include <sharpen/SystemError.hpp>
#include <cassert>

sharpen::WinTimer::WinTimer()
    :Mybase()
    ,handle_(INVALID_HANDLE_VALUE)
    ,future_(nullptr)
    ,tick_(0)
{
    this->handle_ = ::CreateWaitableTimerA(nullptr,TRUE,nullptr);
    if (this->handle_ == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::WinTimer::~WinTimer() noexcept
{
    if (this->handle_ != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(this->handle_);
    }
}

void sharpen::WinTimer::CompleteFuture(void *arg,DWORD,DWORD)
{
    std::unique_ptr<sharpen::WinTimer::WaitStruct> wait(reinterpret_cast<sharpen::WinTimer::WaitStruct*>(arg));
    assert(wait);
    if (wait->timer_->tick_.load() == wait->localTick_)
    {
        sharpen::Future<void> *future;
        std::swap(wait->timer_->future_,future);
        if(future)
        {
            future->Complete();
        }
    }
}

void sharpen::WinTimer::WaitAsync(sharpen::Future<void> &future,sharpen::Uint64 waitMs)
{
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    LARGE_INTEGER li;
    li.QuadPart = -10*1000*waitMs;
    sharpen::WinTimer::WaitStruct *wait = new sharpen::WinTimer::WaitStruct();
    if(wait == nullptr)
    {
        throw std::bad_alloc();
    }
    wait->timer_ = this;
    wait->localTick_ = this->tick_.load();
    this->future_ = &future;
    BOOL r = ::SetWaitableTimer(this->handle_,&li,0,&sharpen::WinTimer::CompleteFuture,wait,FALSE);
    if(r == FALSE)
    {
        delete wait;
        sharpen::ThrowLastError();
    }
}

void sharpen::WinTimer::Cancel()
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