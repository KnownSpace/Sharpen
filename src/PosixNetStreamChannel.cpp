#include <sharpen/PosixNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_POSIXSOCKET

#include <sharpen/EventLoop.hpp>
#include <sharpen/SystemError.hpp>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>

sharpen::PosixNetStreamChannel::PosixNetStreamChannel(sharpen::FileHandle handle)
    : Mybase()
    , readable_(false)
    , writeable_(false)
    , status_(sharpen::PosixNetStreamChannel::IoStatus::Io)
    , reader_()
    , writer_()
    , acceptCb_()
    , connectCb_()
    , pollReadCbs_()
    , pollWriteCbs_() {
    this->handle_ = handle;
    // reset closer before ~IChannel() to be invoked
    // because SafeClose use this pointer
    this->closer_ =
        std::bind(&sharpen::PosixNetStreamChannel::SafeClose, this, std::placeholders::_1);
}

sharpen::PosixNetStreamChannel::~PosixNetStreamChannel() noexcept {
    this->DoCancel(sharpen::ErrorConnectionAborted);
    // reset closer
    {
        std::function<void(sharpen::FileHandle)> tmp;
        std::swap(tmp, this->closer_);
    }
}

void sharpen::PosixNetStreamChannel::SafeClose(sharpen::FileHandle handle) noexcept {
    if (this->loop_) {
        sharpen::CloseFileHandle(handle);
        // FIXME:throw bad alloc
        return this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::DoSafeCancel,
                                                    this,
                                                    sharpen::ErrorConnectionAborted,
                                                    this->shared_from_this()));
    }
    sharpen::CloseFileHandle(handle);
}

bool sharpen::PosixNetStreamChannel::IsAcceptBlock(sharpen::ErrorCode err) noexcept {
    return err == sharpen::ErrorBlocking || err == EINTR || err == EPROTO ||
           err == sharpen::ErrorConnectionAborted;
}

sharpen::FileHandle sharpen::PosixNetStreamChannel::DoAccept() {
    sharpen::FileHandle s;
    do {
        s = ::accept4(this->handle_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    } while (s == -1 && sharpen::GetLastError() == EINTR);
    return s;
}

void sharpen::PosixNetStreamChannel::DoRead() {
    bool blocking;
    bool executed;
    this->reader_.Execute(this->handle_, executed, blocking);
    this->readable_ = !executed || !blocking;
}

void sharpen::PosixNetStreamChannel::DoWrite() {
    bool blocking;
    bool executed;
    this->writer_.Execute(this->handle_, executed, blocking);
    this->writeable_ = !executed || !blocking;
}

void sharpen::PosixNetStreamChannel::DoPollRead() {
    if (this->pollReadCbs_.empty()) {
        return;
    }
    iovec io;
    io.iov_base = nullptr;
    io.iov_len = 0;
    ssize_t size = ::readv(this->handle_, &io, 1);
    for (auto begin = this->pollReadCbs_.begin(); begin != this->pollReadCbs_.end(); ++begin) {
        (*begin)(size);
    }
    this->pollReadCbs_.clear();
}

void sharpen::PosixNetStreamChannel::DoPollWrite() {
    if (this->pollWriteCbs_.empty()) {
        return;
    }
    iovec io;
    io.iov_base = nullptr;
    io.iov_len = 0;
    ssize_t size = ::writev(this->handle_, &io, 1);
    for (auto begin = this->pollWriteCbs_.begin(); begin != this->pollWriteCbs_.end(); ++begin) {
        (*begin)(size);
    }
    this->pollWriteCbs_.clear();
}

void sharpen::PosixNetStreamChannel::HandleAccept() {
    this->readable_ = true;
    if (this->acceptCb_) {
        AcceptCallback cb;
        sharpen::FileHandle accept = this->DoAccept();
        if (accept == -1) {
            sharpen::ErrorCode err = sharpen::GetLastError();
            if (sharpen::PosixNetStreamChannel::IsAcceptBlock(err)) {
                this->readable_ = false;
                return;
            }
        }
        std::swap(this->acceptCb_, cb);
        if (cb) {
            cb(accept);
            return;
        }
        sharpen::CloseFileHandle(accept);
    }
}

void sharpen::PosixNetStreamChannel::HandleRead() {
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Accept) {
        this->HandleAccept();
        return;
    }
    this->DoPollRead();
    this->DoRead();
}

bool sharpen::PosixNetStreamChannel::HandleConnect() {
    int err{0};
    socklen_t errSize = sizeof(err);
    ::getsockopt(this->handle_, SOL_SOCKET, SO_ERROR, &err, &errSize);
    ConnectCallback cb;
    std::swap(cb, this->connectCb_);
    errno = err;
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
    if (cb) {
        cb();
    }
    return err == 0;
}

void sharpen::PosixNetStreamChannel::HandleWrite() {
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Connect) {
        bool continuable = this->HandleConnect();
        if (!continuable) {
            return;
        }
    }
    this->DoPollWrite();
    this->DoWrite();
}

void sharpen::PosixNetStreamChannel::TryRead(char *buf, std::size_t bufSize, Callback cb) {
    this->reader_.AddPendingTask(buf, bufSize, std::move(cb));
    if (this->readable_) {
        this->DoRead();
    }
}

void sharpen::PosixNetStreamChannel::TryWrite(const char *buf, std::size_t bufSize, Callback cb) {
    this->writer_.AddPendingTask(const_cast<char *>(buf), bufSize, std::move(cb));
    if (this->writeable_) {
        this->DoWrite();
    }
}

void sharpen::PosixNetStreamChannel::TryPollRead(Callback cb) {
    this->pollReadCbs_.push_back(std::move(cb));
    if (this->readable_) {
        this->DoPollRead();
    }
}

void sharpen::PosixNetStreamChannel::TryPollWrite(Callback cb) {
    this->pollWriteCbs_.push_back(std::move(cb));
    if (this->writeable_) {
        this->DoPollWrite();
    }
}

void sharpen::PosixNetStreamChannel::TryAccept(AcceptCallback cb) {
    if (this->readable_) {
        sharpen::FileHandle handle = this->DoAccept();
        if (handle == -1) {
            sharpen::ErrorCode err = sharpen::GetLastError();
            if (!sharpen::PosixNetStreamChannel::IsAcceptBlock(err)) {
                cb(handle);
                return;
            }
        } else {
            cb(handle);
            return;
        }
    }
    this->readable_ = false;
    this->acceptCb_ = std::move(cb);
}

void sharpen::PosixNetStreamChannel::TryConnect(const sharpen::IEndPoint &endPoint,
                                                ConnectCallback cb) {
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Connect;
    int r = ::connect(this->handle_, endPoint.GetAddrPtr(), endPoint.GetAddrLen());
    if (r == -1) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != EINPROGRESS && err != EINTR) {
            this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
            cb();
            return;
        }
        this->connectCb_ = std::move(cb);
        return;
    }
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
    cb();
}

void sharpen::PosixNetStreamChannel::RequestRead(char *buf,
                                                 std::size_t bufSize,
                                                 sharpen::Future<std::size_t> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<std::size_t> *, ssize_t);
    Callback cb = std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),
                            this->loop_,
                            future,
                            std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixNetStreamChannel::TryRead, this, buf, bufSize, std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestWrite(const char *buf,
                                                  std::size_t bufSize,
                                                  sharpen::Future<std::size_t> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<std::size_t> *, ssize_t);
    Callback cb = std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),
                            this->loop_,
                            future,
                            std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixNetStreamChannel::TryWrite, this, buf, bufSize, std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestSendFile(sharpen::FileHandle handle,
                                                     std::uint64_t offset,
                                                     std::size_t size,
                                                     sharpen::Future<std::size_t> *future) {
    std::size_t memSize = size;
    std::size_t over = offset % 4096;
    if (over) {
        offset -= over;
    }
    memSize += over;
    over = memSize % 4096;
    if (over) {
        memSize += (4096 - over);
    }
    void *mem = ::mmap64(nullptr, memSize, PROT_READ, MAP_SHARED, handle, offset);
    if (mem == nullptr) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    std::uintptr_t p = reinterpret_cast<std::uintptr_t>(mem);
    p += over;
    using FnPtr = void (*)(
        sharpen::EventLoop *, sharpen::Future<std::size_t> *, void *, std::size_t, ssize_t);
    Callback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteSendFileCallback),
                  this->loop_,
                  future,
                  mem,
                  memSize,
                  std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&sharpen::PosixNetStreamChannel::TryWrite,
                                     this,
                                     reinterpret_cast<const char *>(p),
                                     size,
                                     std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestConnect(const sharpen::IEndPoint &endPoint,
                                                    sharpen::Future<void> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<void> *);
    ConnectCallback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteConnectCallback),
                  this->loop_,
                  future);
    this->loop_->RunInLoop(std::bind(
        &sharpen::PosixNetStreamChannel::TryConnect, this, std::cref(endPoint), std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestAccept(
    sharpen::Future<sharpen::NetStreamChannelPtr> *future) {
    using FnPtr = void (*)(
        sharpen::EventLoop *, sharpen::Future<sharpen::NetStreamChannelPtr> *, sharpen::FileHandle);
    AcceptCallback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteAcceptCallback),
                  this->loop_,
                  future,
                  std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixNetStreamChannel::TryAccept, this, std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestPollRead(sharpen::Future<void> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<void> *, ssize_t);
    Callback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompletePollCallback),
                  this->loop_,
                  future,
                  std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixNetStreamChannel::TryPollRead, this, std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestPollWrite(sharpen::Future<void> *future) {
    using FnPtr = void (*)(sharpen::EventLoop *, sharpen::Future<void> *, ssize_t);
    Callback cb =
        std::bind(static_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompletePollCallback),
                  this->loop_,
                  future,
                  std::placeholders::_1);
    this->loop_->RunInLoop(
        std::bind(&sharpen::PosixNetStreamChannel::TryPollWrite, this, std::move(cb)));
}

void sharpen::PosixNetStreamChannel::CompleteConnectCallback(
    sharpen::EventLoop *loop, sharpen::Future<void> *future) noexcept {
    if (sharpen::GetLastError() != 0) {
        // connect error
        loop->RunInLoopSoon(
            std::bind(&sharpen::Future<void>::Fail, future, sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<void>::CompleteForBind, future));
}

void sharpen::PosixNetStreamChannel::CompleteIoCallback(sharpen::EventLoop *loop,
                                                        sharpen::Future<std::size_t> *future,
                                                        ssize_t size) noexcept {
    if (size == -1) {
        sharpen::ErrorCode code{sharpen::GetLastError()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorConnectionAborted ||
            code == sharpen::ErrorConnectionReset) {
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

void sharpen::PosixNetStreamChannel::CompletePollCallback(sharpen::EventLoop *loop,
                                                          sharpen::Future<void> *future,
                                                          ssize_t size) noexcept {
    if (size == -1) {
        loop->RunInLoopSoon(
            std::bind(&sharpen::Future<void>::Fail, future, sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<void>::CompleteForBind, future));
}

void sharpen::PosixNetStreamChannel::CompleteSendFileCallback(sharpen::EventLoop *loop,
                                                              sharpen::Future<std::size_t> *future,
                                                              void *mem,
                                                              std::size_t memLen,
                                                              ssize_t size) noexcept {
    ::munmap(mem, memLen);
    if (size == -1) {
        sharpen::ErrorCode code{sharpen::GetLastError()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorConnectionAborted ||
            code == sharpen::ErrorConnectionReset) {
            loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::CompleteForBind,
                                          future,
                                          static_cast<std::size_t>(0)));
        }
        loop->RunInLoopSoon(
            std::bind(&sharpen::Future<std::size_t>::Fail, future, sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(
        &sharpen::Future<std::size_t>::CompleteForBind, future, static_cast<std::size_t>(size)));
}

void sharpen::PosixNetStreamChannel::CompleteAcceptCallback(
    sharpen::EventLoop *loop,
    sharpen::Future<sharpen::NetStreamChannelPtr> *future,
    sharpen::FileHandle accept) noexcept {
    if (accept == -1) {
        loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::NetStreamChannelPtr>::Fail,
                                      future,
                                      sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::NetStreamChannelPtr>::CompleteForBind,
                                  future,
                                  std::make_shared<sharpen::PosixNetStreamChannel>(accept)));
}


void sharpen::PosixNetStreamChannel::WriteAsync(const char *buf,
                                                std::size_t bufSize,
                                                sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == -1) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->RequestWrite(buf, bufSize, &future);
}

void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf,
                                                std::size_t bufferOffset,
                                                sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(char *buf,
                                               std::size_t bufSize,
                                               sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == -1) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->RequestRead(buf, bufSize, &future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf,
                                               std::size_t bufferOffset,
                                               sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, future);
}

void sharpen::PosixNetStreamChannel::OnEvent(sharpen::IoEvent *event) {
    if (event->IsReadEvent() || event->IsErrorEvent()) {
        this->HandleRead();
    }
    if (event->IsWriteEvent() || event->IsErrorEvent()) {
        this->HandleWrite();
    }
}

void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,
                                                   std::uint64_t size,
                                                   std::uint64_t offset,
                                                   sharpen::Future<std::size_t> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == -1) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->RequestSendFile(file->GetHandle(), offset, size, &future);
}

void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,
                                                   sharpen::Future<std::size_t> &future) {
    this->SendFileAsync(file, file->GetFileSize(), 0, future);
}

void sharpen::PosixNetStreamChannel::AcceptAsync(
    sharpen::Future<sharpen::NetStreamChannelPtr> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == -1) {
        future.Fail(sharpen::MakeSystemErrorPtr(sharpen::ErrorCancel));
        return;
    }
    this->RequestAccept(&future);
}

void sharpen::PosixNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint,
                                                  sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == -1) {
        future.Fail(sharpen::MakeSystemErrorPtr(sharpen::ErrorConnectionAborted));
        return;
    }
    this->RequestConnect(endpoint, &future);
}

void sharpen::PosixNetStreamChannel::PollReadAsync(sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestPollRead(&future);
}

void sharpen::PosixNetStreamChannel::PollWriteAsync(sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestPollWrite(&future);
}

void sharpen::PosixNetStreamChannel::Listen(std::uint16_t queueLength) {
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Accept;
    Mybase::Listen(queueLength);
}

void sharpen::PosixNetStreamChannel::DoCancel(sharpen::ErrorCode err) noexcept {
    // cancel all io
    this->reader_.CancelAllIo(err);
    this->writer_.CancelAllIo(err);
    errno = err;
    for (auto begin = this->pollReadCbs_.begin(); begin != this->pollReadCbs_.end(); ++begin) {
        (*begin)(-1);
    }
    this->pollReadCbs_.clear();
    for (auto begin = this->pollWriteCbs_.begin(); begin != this->pollWriteCbs_.end(); ++begin) {
        (*begin)(-1);
    }
    this->pollWriteCbs_.clear();
    AcceptCallback acb;
    std::swap(this->acceptCb_, acb);
    if (acb) {
        acb(-1);
    }
    ConnectCallback ccb;
    std::swap(this->connectCb_, ccb);
    if (ccb) {
        ccb();
    }
}

void sharpen::PosixNetStreamChannel::DoSafeCancel(sharpen::ErrorCode err,
                                                  sharpen::ChannelPtr keepalive) noexcept {
    (void)keepalive;
    this->DoCancel(err);
}

void sharpen::PosixNetStreamChannel::Cancel() noexcept {
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::DoSafeCancel,
                                         this,
                                         sharpen::ErrorCancel,
                                         this->shared_from_this()));
}

#endif