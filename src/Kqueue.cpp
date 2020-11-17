#include <sharpen/Kqueue.hpp>
#ifdef SHARPEN_HAS_KQUEUE

#include <sharpen/SystemError.hpp>

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
    timespec *timeoutPtr = nullptr;
    timespec timeoutSpec;
    if(timeout != -1)
    {
        time_t sec = timeout / 1000);
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

void sharpen::Kqueue::Add(sharpen::FileHandle handle,sharpen::Int16 eventType,void *data)
{}

void sharpen::Kqueue::Remove(sharpen::FileHandle handle)
{}

void sharpen::Kqueue::Update(sharpen::FileHandle handle,sharpen::Int16 eventType,void *data)
{}
#endif
