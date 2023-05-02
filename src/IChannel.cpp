#include <sharpen/IChannel.hpp>

#include <sharpen/IEventLoopGroup.hpp>
#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_NIX
#include <unistd.h>
#endif

sharpen::IChannel::~IChannel() noexcept {
    this->Close();
}

void sharpen::IChannel::Register(sharpen::EventLoop &loop) {
    loop.Bind(this->shared_from_this());
    this->loop_ = &loop;
}

void sharpen::IChannel::Register(sharpen::IEventLoopGroup &loopGroup) {
    sharpen::EventLoop &loop = loopGroup.RoundRobinLoop();
    this->Register(loop);
}

void sharpen::CloseFileHandle(sharpen::FileHandle handle) noexcept {
#ifdef SHARPEN_IS_WIN
    ::CloseHandle(handle);
#else
    ::close(handle);
#endif
}

void sharpen::IChannel::Close() noexcept {
#ifdef SHARPEN_IS_WIN
    const sharpen::FileHandle invalidHandle{static_cast<sharpen::FileHandle>(INVALID_HANDLE_VALUE)};
#else
    constexpr sharpen::FileHandle invalidHandle{-1};
#endif
    sharpen::FileHandle handle{invalidHandle};
    std::swap(this->handle_, handle);
    if (handle != invalidHandle) {
        if (this->closer_) {
            this->closer_(handle);
        } else {
            sharpen::CloseFileHandle(handle);
        }
    }
}

bool sharpen::IChannel::IsClosed() const noexcept {
#ifdef SHARPEN_IS_WIN
    return this->handle_ == INVALID_HANDLE_VALUE;
#else
    return this->handle_ == -1;
#endif
}