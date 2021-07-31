#include <sharpen/WinTimer.hpp>

#ifdef SHARPEN_HAS_WAITABLETIMER

#include <Windows.h>
#include <sharpen/SystemError.hpp>

sharpen::WinTimer::WinTimer()
    :Mybase()
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
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void>*>(arg);
    future->Complete();
}

void sharpen::WinTimer::WaitAsync(sharpen::Future<void> &future,sharpen::Uint64 waitMs)
{
    LARGE_INTEGER li;
    li.QuadPart = -10*1000*waitMs;
    ::SetWaitableTimer(this->handle_,&li,0,&sharpen::WinTimer::CompleteFuture,&future,FALSE);
}

#endif