#include <sharpen/Epoll.hpp>
#ifdef SHARPEN_HAS_EPOLL

#include <sharpen/SystemError.hpp>
#include <cassert>

sharpen::Epoll::Epoll()
    : handle_(-1)
{
    this->handle_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (this->handle_ == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::Epoll::~Epoll() noexcept
{
    if (this->handle_ != -1)
    {
        ::close(this->handle_);
    }
}

std::uint32_t sharpen::Epoll::Wait(sharpen::Epoll::Event *events,
                                   std::uint32_t maxEvent,
                                   std::int32_t timeout)
{
    assert(this->handle_ != -1);
    assert(static_cast<std::int32_t>(maxEvent) >= 0);
    int r = ::epoll_wait(this->handle_, events, static_cast<std::int32_t>(maxEvent), timeout);
    if (r == -1)
    {
        if (sharpen::GetLastError() == EINTR)
        {
            return 0;
        }
        sharpen::ThrowLastError();
    }
    return static_cast<std::uint32_t>(r);
}

void sharpen::Epoll::Add(sharpen::FileHandle handle, sharpen::Epoll::Event *event)
{
    assert(this->handle_ != -1);
    assert(event != nullptr);
    int r = ::epoll_ctl(this->handle_, EPOLL_CTL_ADD, handle, event);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::Epoll::Remove(sharpen::FileHandle handle)
{
    assert(this->handle_ != -1);
    sharpen::Epoll::Event ignore;
    int r = ::epoll_ctl(this->handle_, EPOLL_CTL_DEL, handle, &ignore);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::Epoll::Update(sharpen::FileHandle handle, sharpen::Epoll::Event *event)
{
    assert(this->handle_ != -1);
    assert(event != nullptr);
    int r = ::epoll_ctl(this->handle_, EPOLL_CTL_MOD, handle, event);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}
#endif
