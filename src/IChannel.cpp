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
    if (this->handle_ != INVALID_HANDLE_VALUE)
    {
        if (this->closer_)
        {
            this->closer_(this->handle_);
        }
        else
        {
            ::CloseHandle(this->handle_);
        }
        
        this->handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (this->handle_ != -1)
    {
        if (this->closer_)
        {
            this->closer_(this->handle_);
        }
        else
        {
            ::close(this->handle_);
        }
        this->handle_ = -1;
    }
#endif
}