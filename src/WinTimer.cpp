#include <sharpen/WinTimer.hpp>

#ifdef SHARPEN_HAS_WAITABLETIMER

#include <Windows.h>
#include <sharpen/SystemError.hpp>
#include <cassert>

sharpen::WinTimer::WinTimer()
    :Mybase()
    ,handle_(INVALID_HANDLE_VALUE)
    ,future_(nullptr)
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
    assert(arg);
    sharpen::Future<void> *future = nullptr;
    sharpen::WinTimer *thiz = reinterpret_cast<sharpen::WinTimer*>(arg);
    std::swap(future,thiz->future_);
    if(future)
    {
        future->Complete();
    }
}

void sharpen::WinTimer::WaitAsync(sharpen::Future<void> &future,sharpen::Uint64 waitMs)
{
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    LARGE_INTEGER li;
    li.QuadPart = -10*1000*waitMs;
    this->future_ = &future;
    BOOL r = ::SetWaitableTimer(this->handle_,&li,0,&sharpen::WinTimer::CompleteFuture,this,FALSE);
    if(r == FALSE)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::WinTimer::Cancel()
{
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    ::CancelWaitableTimer(this->handle_);
    sharpen::Future<void> *future = nullptr;
    std::swap(future,this->future_);
    if(future)
    {
        future->Complete();
    }
}

#endif