#include <sharpen/WinTimer.hpp>

#ifdef SHARPEN_HAS_WAITABLETIMER

#include <sharpen/SystemError.hpp>
#include <Windows.h>
#include <cassert>

sharpen::WinTimer::WinTimer()
    : Mybase()
    , handle_(INVALID_HANDLE_VALUE)
    , future_(nullptr) {
    this->handle_ = ::CreateWaitableTimerA(nullptr, TRUE, nullptr);
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        sharpen::ThrowLastError();
    }
}

sharpen::WinTimer::~WinTimer() noexcept {
    if (this->handle_ != INVALID_HANDLE_VALUE) {
        ::CloseHandle(this->handle_);
    }
}

void WINAPI sharpen::WinTimer::CompleteFuture(void *arg, DWORD, DWORD) {
    assert(arg);
    sharpen::WinTimer *thiz = reinterpret_cast<sharpen::WinTimer *>(arg);
    sharpen::Future<bool> *future{thiz->future_.exchange(nullptr)};
    if (future) {
        future->Complete(true);
    }
}

void sharpen::WinTimer::WaitAsync(sharpen::Future<bool> &future, std::uint64_t waitMs) {
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    if (waitMs == 0) {
        future.Complete(true);
        return;
    }
    LARGE_INTEGER li;
    li.QuadPart = -10 * 1000 * waitMs;
    this->future_ = &future;
    BOOL r =
        ::SetWaitableTimer(this->handle_, &li, 0, &sharpen::WinTimer::CompleteFuture, this, FALSE);
    if (r == FALSE) {
        sharpen::ThrowLastError();
    }
}

void sharpen::WinTimer::Cancel() {
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    if (!this->future_) {
        return;
    }
    ::CancelWaitableTimer(this->handle_);
    sharpen::Future<bool> *future{this->future_.exchange(nullptr)};
    if (future) {
        future->Complete(false);
    }
}

#endif