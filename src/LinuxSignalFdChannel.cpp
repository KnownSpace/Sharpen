#include <sharpen/LinuxSignalFdChannel.hpp>

#ifdef SHARPEN_HAS_SIGNALFD

#include <cassert>

#include <sharpen/EventLoop.hpp>
#include <sharpen/IoEvent.hpp>
#include <sharpen/SystemError.hpp>

sharpen::LinuxSignalFdChannel::LinuxSignalFdChannel(sharpen::FileHandle handle)
    : Base()
    , bufMap_(0)
    , readable_(false)
{
    this->handle_ = handle;
    assert(this->handle_ != -1);
    this->closer_ = std::bind(&Self::SafeClose, this, std::placeholders::_1);
}

void sharpen::LinuxSignalFdChannel::DoSafeClose(sharpen::ErrorCode err,
                                                sharpen::ChannelPtr keepalive) noexcept
{
    (void)keepalive;
    for (auto begin = this->tasks_.begin(), end = this->tasks_.end(); begin != end; ++begin)
    {
        errno = err;
        begin->cb(-1);
    }
}

void sharpen::LinuxSignalFdChannel::SafeClose(sharpen::FileHandle handle) noexcept
{
    if (this->loop_)
    {
        sharpen::CloseFileHandle(handle);
        // FIXME:throw bad alloc
        return this->loop_->RunInLoopSoon(std::bind(
            &Self::DoSafeClose, this, sharpen::ErrorBrokenPipe, this->shared_from_this()));
    }
    sharpen::CloseFileHandle(handle);
}

sharpen::LinuxSignalFdChannel::~LinuxSignalFdChannel() noexcept
{
    std::function<void(sharpen::FileHandle)> closer;
    std::swap(this->closer_, closer);
    for (auto begin = this->tasks_.begin(), end = this->tasks_.end(); begin != end; ++begin)
    {
        errno = sharpen::ErrorBrokenPipe;
        begin->cb(-1);
    }
}

std::uint8_t sharpen::LinuxSignalFdChannel::PopSignal() noexcept
{
    if (this->bufMap_ != 0)
    {
        for (std::size_t i = 0; i != sizeof(this->bufMap_) * 8; ++i)
        {
            std::uint64_t pos{1};
            pos <<= i;
            if (this->bufMap_ & pos)
            {
                this->bufMap_ &= ~pos;
                return static_cast<std::uint8_t>(i + 1);
            }
        }
    }
    return 0;
}

void sharpen::LinuxSignalFdChannel::DoRead()
{
    signalfd_siginfo buf;
    bool closed{false};
    sharpen::ErrorCode err{0};
    while (this->readable_)
    {
        ssize_t size;
        do {
            size = sharpen::ReadSignalFd(this->handle_, buf);
        } while (size == -1 && sharpen::GetLastError() == EINTR);
        if (!size)
        {
            // signal fd closed
            closed = true;
            break;
        }
        if (size == -1)
        {
            if (sharpen::GetLastError() == sharpen::ErrorBlocking)
            {
                this->readable_ = false;
            }
            else
            {
                err = sharpen::GetLastError();
            }
            break;
        }
        assert(size == sizeof(buf));
        std::uint64_t pos{1};
        assert(buf.ssi_signo > 0);
        pos <<= buf.ssi_signo - 1;
        this->bufMap_ |= pos;
    }
    // handle tasks
    if (closed || err)
    {
        Tasks tasks;
        std::swap(tasks, this->tasks_);
        ssize_t sz{0};
        if (err)
        {
            errno = err;
            sz = -1;
        }
        for (auto begin = tasks.begin(), end = tasks.end(); begin != end; ++begin)
        {
            begin->cb(sz);
        }
    }
    else
    {
        for (auto begin = this->tasks_.begin(), end = this->tasks_.end(); begin != end; ++begin)
        {
            std::size_t offset{0};
            while (offset != begin->bufSize)
            {
                std::uint8_t signal{this->PopSignal()};
                if (!signal)
                {
                    break;
                }
                begin->buf[offset] = signal;
                ++offset;
            }
            if (!offset)
            {
                break;
            }
            // task completed
            begin->cb(offset);
            begin = this->tasks_.erase(begin);
        }
    }
}

void sharpen::LinuxSignalFdChannel::HandleRead()
{
    this->readable_ = true;
    this->DoRead();
}

void sharpen::LinuxSignalFdChannel::OnEvent(sharpen::IoEvent *event)
{
    assert(event != nullptr);
    if (event->IsReadEvent() || event->IsCloseEvent() || event->IsErrorEvent())
    {
        this->HandleRead();
    }
}

void sharpen::LinuxSignalFdChannel::TryRead(char *buf, std::size_t bufSize, Callback cb)
{
    ReadTask task;
    task.buf = buf;
    task.bufSize = bufSize;
    task.cb = std::move(cb);
    this->tasks_.emplace_back(std::move(task));
    if (this->readable_)
    {
        this->DoRead();
    }
}

void sharpen::LinuxSignalFdChannel::CompleteReadCallback(sharpen::EventLoop *loop,
                                                         sharpen::Future<std::size_t> *future,
                                                         ssize_t size) noexcept
{
    if (size == -1)
    {
        sharpen::ErrorCode code{sharpen::GetLastError()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorBrokenPipe)
        {
            loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::CompleteForBind,
                                          future,
                                          static_cast<std::size_t>(0)));
            return;
        }
        loop->RunInLoopSoon(std::bind(
            &sharpen::Future<std::size_t>::Fail, future, sharpen::MakeSystemErrorPtr(code)));
        return;
    }
    loop->RunInLoopSoon(std::bind(
        &sharpen::Future<std::size_t>::CompleteForBind, future, static_cast<std::size_t>(size)));
}

void sharpen::LinuxSignalFdChannel::RequestRead(char *buf,
                                                std::size_t bufSize,
                                                sharpen::Future<std::size_t> *future)
{
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<std::size_t> *, ssize_t);
    Callback cb = std::bind(static_cast<FnPtr>(&Self::CompleteReadCallback),
                            this->loop_,
                            future,
                            std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&Self::TryRead, this, buf, bufSize, std::move(cb)));
}

void sharpen::LinuxSignalFdChannel::ReadAsync(sharpen::SignalBuffer &signals,
                                              sharpen::Future<std::size_t> &future)
{
    assert(signals.Data() != nullptr || (signals.Data() == nullptr && signals.GetSize() == 0));
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestRead(signals.Data(), signals.GetSize(), &future);
}
#endif