#include <sharpen/Kqueue.hpp>
#ifdef SHARPEN_HAS_KQUEUE

#include <sharpen/SystemError.hpp>
#include <cassert>

sharpen::Kqueue::Kqueue()
    :handle_(::kqueue())
{
    if(this->handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

~sharpen::Kqueue::Kqueue() noexcept
{
    ::close(this->handle_);
}  

sharpen::Uint32 sharpen::Kqueue::Wait(sharpen::Kqueue::Event *events,sharpen::Int32 maxEvent,int timeout)
{
    assert(this->handle_ != -1);
    timespec *timeoutPtr = nullptr;
    timespec timeoutSpec;
    if(timeout != -1)
    {
        time_t sec = timeout / 1000;
        timeout %= 1000;
        timeoutSpec.tv_sec = sec;
        timeoutSpec.tv_nsec = timeout * 1000 * 1000;
        timeoutPtr = &timeoutSpec;
    }
    sharpen::Int32 r = ::kevent(this->handle_,nullptr,0,events,maxEvent,timeoutPtr);
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
    return r;
}

void sharpen::Kqueue::Add(sharpen::FileHandle handle,sharpen::Int16 eventType,sharpen::Uint32 fflags,sharpen::Int64 data,void *udata)
{
    assert(this->handle_ != -1);
}

void sharpen::Kqueue::Remove(sharpen::FileHandle handle)
{
    assert(this->handle_ != -1);
}

void sharpen::Kqueue::Update(sharpen::FileHandle handle,sharpen::Int16 eventType,sharpen::Uint32 fflags,sharpen::Int64 data,void *udata)
{
    assert(this->handle_ != -1);
}
#endif
