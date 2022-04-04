#include <sharpen/IChannel.hpp>
#include <sharpen/EventEngine.hpp>

#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_NIX
#include <unistd.h>
#endif

sharpen::IChannel::~IChannel() noexcept
{
    this->Close();
}

void sharpen::IChannel::Register(sharpen::EventLoop *loop)
{
    loop->Bind(this->shared_from_this());
    this->loop_ = loop;
}

void sharpen::IChannel::Register(sharpen::EventEngine &engine)
{
    sharpen::EventLoop *loop = engine.RoundRobinLoop();
    this->Register(loop);
}

void sharpen::IChannel::Close() noexcept
{
#ifdef SHARPEN_IS_WIN
    sharpen::FileHandle handle{INVALID_HANDLE_VALUE};
    std::swap(this->handle_,handle);
    if (handle != INVALID_HANDLE_VALUE)
    {
        if (this->closer_)
        {
            this->closer_(handle);
        }
        else
        {
            ::CloseHandle(handle);
        }
    }
#else
    sharpen::FileHandle handle{-1};
    std::swap(this->handle_,handle);
    if (handle != -1)
    {
        if (this->closer_)
        {
            this->closer_(handle);
        }
        else
        {
            ::close(handle);
        }
    }
#endif
}

bool sharpen::IChannel::IsClosed() const noexcept
{
#ifdef SHARPEN_IS_WIN
    return this->handle_ == INVALID_HANDLE_VALUE;
#else
    return this->handle_ == -1;
#endif
}