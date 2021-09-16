#include <sharpen/EventFd.hpp>

#ifdef SHARPEN_HAS_EVENTFD

#include <unistd.h>
#include <sys/eventfd.h>
#include <cassert>

#include <sharpen/SystemError.hpp>

sharpen::EventFd::EventFd(sharpen::Uint32 initVal,int flags)
    :handle_(::eventfd(initVal,flags))
{
    if(handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::EventFd::EventFd(sharpen::EventFd &&other) noexcept
    :handle_(other.handle_)
{
    other.handle_ = -1;
}

sharpen::EventFd::~EventFd() noexcept
{
    if(this->handle_ != -1)
    {
        ::close(this->handle_);
    }
}

sharpen::EventFd::EventFdValue sharpen::EventFd::Read()
{
    assert(this->handle_ != -1);
    sharpen::EventFd::EventFdValue val;
    int r = ::read(this->handle_,&val,sizeof(val));
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
    return val;
}

void sharpen::EventFd::Write(sharpen::EventFd::EventFdValue value)
{
    assert(this->handle_ != -1);
    int r = ::write(this->handle_,&value,sizeof(value));
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::FileHandle sharpen::EventFd::GetHandle() noexcept
{
    return this->handle_;
}

sharpen::EventFd &sharpen::EventFd::operator=(sharpen::EventFd &&other) noexcept
{
    if(this == std::addressof(other))
    {
        return *this;
    }
    this->handle_ = other.handle_;
    other.handle_ = -1;
    return *this;
}

#endif
