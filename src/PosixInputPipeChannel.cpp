#include <sharpen/PosixInputPipeChannel.hpp>
#ifdef SHARPEN_HAS_POSIXINPUTPIPE

#include <sharpen/EventLoop.hpp>
#include <cassert>
#include <stdexcept>

sharpen::PosixInputPipeChannel::PosixInputPipeChannel(sharpen::FileHandle handle)
    : Mybase()
    , reader_()
    , readable_(false)
    , peerClosed_(false) {
    assert(handle != -1);
    this->handle_ = handle;
    this->closer_ = std::bind(&Self::SafeClose, this, std::placeholders::_1);
}

sharpen::PosixInputPipeChannel::~PosixInputPipeChannel() noexcept {
    std::function<void(sharpen::FileHandle)> closer;
    std::swap(closer, this->closer_);
    this->reader_.CancelAllIo(sharpen::ErrorBrokenPipe);
}

void sharpen::PosixInputPipeChannel::DoSafeClose(sharpen::ErrorCode err,
                                                 sharpen::ChannelPtr keepalive) noexcept {
    (void)keepalive;
    this->reader_.CancelAllIo(err);
}

void sharpen::PosixInputPipeChannel::SafeClose(sharpen::FileHandle handle) noexcept {
    if (this->loop_) {
        sharpen::CloseFileHandle(handle);
        // FIXME:throw bad alloc
        return this->loop_->RunInLoopSoon(std::bind(
            &Self::DoSafeClose, this, sharpen::ErrorBrokenPipe, this->shared_from_this()));
    }
    sharpen::CloseFileHandle(handle);
}

void sharpen::PosixInputPipeChannel::HandleRead() {
    this->DoRead();
}

void sharpen::PosixInputPipeChannel::DoRead() {
    bool executed;
    bool blocking;
    this->reader_.Execute(this->handle_, executed, blocking);
    this->readable_ = !executed || !blocking;
    if (!this->readable_ && this->peerClosed_) {
        this->reader_.CancelAllIo(sharpen::ErrorBrokenPipe);
    }
}

void sharpen::PosixInputPipeChannel::TryRead(char *buf, std::size_t bufSize, Callback cb) {
    this->reader_.AddPendingTask(buf, bufSize, std::move(cb));
    if (this->readable_ || this->peerClosed_) {
        this->DoRead();
    }
}

void sharpen::PosixInputPipeChannel::RequestRead(char *buf,
                                                 std::size_t bufSize,
                                                 sharpen::Future<std::size_t> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<std::size_t> *, ssize_t);
    Callback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixInputPipeChannel::CompleteReadCallback),
                  this->loop_,
                  future,
                  std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixInputPipeChannel::TryRead, this, buf, bufSize, std::move(cb)));
}

void sharpen::PosixInputPipeChannel::ReadAsync(char *buf,
                                               std::size_t bufSize,
                                               sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if(bufSize > MaxIoSize) {
        bufSize = MaxIoSize;
    }
    this->RequestRead(buf, bufSize, &future);
}

void sharpen::PosixInputPipeChannel::ReadAsync(sharpen::ByteBuffer &buf,
                                               std::size_t bufOffset,
                                               sharpen::Future<std::size_t> &future) {
    if (bufOffset > buf.GetSize()) {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufOffset, buf.GetSize() - bufOffset, future);
}

void sharpen::PosixInputPipeChannel::OnEvent(sharpen::IoEvent *event) {
    if (event->IsReadEvent()) {
        this->HandleRead();
    }
    if (event->IsCloseEvent()) {
        this->peerClosed_ = true;
        this->reader_.CancelAllIo(sharpen::ErrorBrokenPipe);
    }
}

void sharpen::PosixInputPipeChannel::CompleteReadCallback(sharpen::EventLoop *loop,
                                                          sharpen::Future<std::size_t> *future,
                                                          ssize_t size) noexcept {
    if (size == -1) {
        sharpen::ErrorCode code{sharpen::GetLastError()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorBrokenPipe) {
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
#endif