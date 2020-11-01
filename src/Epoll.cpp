#include <sharpen/Epoll.hpp>
#ifdef SHARPEN_HAS_EPOLL

#include <cassert>

#include <sharpen/SystemError.hpp>

sharpen::Epoll::Epoll()
    :handle_(::epoll_create1(EPOLL_CLOEXEC))
{
    if(this->handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::Epoll::~Epoll() noexcept
{
    if(this->handle_ != -1)
    {
        ::close(this->handle_);
    }
}

sharpen::Uint32 sharpen::Epoll::Wait(sharpen::Epoll::Event *events,sharpen::Int32 maxEvent,int timeout)
{
    assert(this->handle_ != -1);
    int r = ::epoll_wait(this->handle_,events,maxEvent,timeout);
    if(r < 0)
    {
        sharpen::ThrowLastError();
    }
    return static_cast<sharpen::Uint32>(r);
}

void sharpen::Epoll::Add(sharpen::FileHandle handle,sharpen::Epoll::Event *event)
{
    assert(this->handle_ != -1);
    int r = ::epoll_ctl(this->handle_,EPOLL_CTL_ADD,handle,event);
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::Epoll::Remove(sharpen::FileHandle handle)
{
    assert(this->handle_ != -1);
    sharpen::Epoll::Event ignore;
    int r = ::epoll_ctl(this->handle_,EPOLL_CTL_DEL,handle,&ignore);
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::Epoll::Update(sharpen::FileHandle handle,sharpen::Epoll::Event *event)
{
    assert(this->handle_ != -1);
    int r = ::epoll_ctl(this->handle_,EPOLL_CTL_MOD,handle,event);
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}
#endif
