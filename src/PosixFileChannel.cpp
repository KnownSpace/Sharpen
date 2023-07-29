#include <sharpen/PosixFileChannel.hpp>

#ifdef SHARPEN_HAS_POSIXFILE

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>

#include <sharpen/EventLoop.hpp>
#include <sharpen/SystemError.hpp>


#if (defined SHARPEN_ON_WSL) && (defined SHARPEN_HAS_IOURING)
// if we on WSL, disable io_uring
// some operations are not supported on WSL
#define SHARPEN_FORCE_NORMAL_FILE_IO
#endif

#ifdef SHARPEN_HAS_IOURING
#include <cstring>
#include <new>

#include <sharpen/EpollSelector.hpp>
#endif

sharpen::PosixFileChannel::PosixFileChannel(sharpen::FileHandle handle, bool syncWrite)
    : MyBase()
#ifdef SHARPEN_HAS_IOURING
    , queue_(nullptr)
#endif
    , syncWrite_(syncWrite) {
    assert(handle != -1);
    this->handle_ = handle;
}

void sharpen::PosixFileChannel::NormalWrite(const char *buf,
                                            std::size_t bufSize,
                                            std::uint64_t offset,
                                            sharpen::Future<std::size_t> *future) {
    ssize_t r;
    do {
        r = ::pwrite64(this->handle_, buf, bufSize, offset);
    } while (r == -1 && sharpen::GetLastError() == EINTR);
    if (r == -1) {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(static_cast<std::size_t>(r));
}

void sharpen::PosixFileChannel::NormalAllocate(std::uint64_t offset,
                                               std::size_t size,
                                               sharpen::Future<std::size_t> *future) {
    int r = ::fallocate64(this->handle_, FALLOC_FL_KEEP_SIZE, offset, size);
    while (r == -1 && sharpen::GetLastError() == EINTR) {
        r = ::fallocate64(this->handle_, FALLOC_FL_KEEP_SIZE, offset, size);
    }
    if (r == -1) {
        future->Fail(sharpen::MakeLastErrorPtr());
    } else {
        future->Complete(size);
    }
}

void sharpen::PosixFileChannel::NormalDeallocate(std::uint64_t offset,
                                                 std::size_t size,
                                                 sharpen::Future<std::size_t> *future) {
    int r = ::fallocate64(this->handle_, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset, size);
    while (r == -1 && sharpen::GetLastError() == EINTR) {
        r = ::fallocate64(this->handle_, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset, size);
    }
    if (r == -1) {
        future->Fail(sharpen::MakeLastErrorPtr());
    } else {
        future->Complete(size);
    }
}

#ifdef SHARPEN_HAS_IOURING

sharpen::IoUringStruct *sharpen::PosixFileChannel::InitStruct(
    void *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future) {
    sharpen::IoUringStruct *st = new (std::nothrow) sharpen::IoUringStruct();
    if (!st) {
        return nullptr;
    }
    st->channel_ = this->shared_from_this();
    st->data_ = future;
    st->length_ = 0;
    st->vec_.iov_base = buf;
    st->vec_.iov_len = bufSize;
    st->event_.SetChannel(this->shared_from_this());
    st->event_.SetEvent(sharpen::IoEvent::EventTypeEnum::None);
    st->event_.SetData(st);
    return st;
}

sharpen::IoUringStruct *sharpen::PosixFileChannel::InitStruct(sharpen::Future<void> *future) {
    sharpen::IoUringStruct *st = new (std::nothrow) sharpen::IoUringStruct();
    if (!st) {
        return nullptr;
    }
    st->channel_ = this->shared_from_this();
    st->data_ = future;
    st->length_ = 0;
    st->vec_.iov_base = nullptr;
    st->vec_.iov_len = 0;
    st->event_.SetChannel(this->shared_from_this());
    st->event_.SetEvent(sharpen::IoEvent::EventTypeEnum::None);
    st->event_.SetData(st);
    return st;
}

#endif

void sharpen::PosixFileChannel::DoWrite(const char *buf,
                                        std::size_t bufSize,
                                        std::uint64_t offset,
                                        sharpen::Future<std::size_t> *future) {
#if (defined SHARPEN_HAS_IOURING) && !(defined SHARPEN_FORCE_NORMAL_FILE_IO)
    assert(this->queue_);
    auto *st = this->InitStruct(const_cast<char *>(buf), bufSize, future);
    if (!st) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe, 0, sizeof(sqe));
    sqe.opcode = IORING_OP_WRITEV;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Write);
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    sqe.addr = reinterpret_cast<std::uint64_t>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalWrite(buf, bufSize, offset, future);
#endif
}

void sharpen::PosixFileChannel::NormalRead(char *buf,
                                           std::size_t bufSize,
                                           std::uint64_t offset,
                                           sharpen::Future<std::size_t> *future) {
    ssize_t r;
    do {
        r = ::pread64(this->handle_, buf, bufSize, offset);
    } while (r == -1 && sharpen::GetLastError() == EINTR);
    if (r == -1) {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(static_cast<std::size_t>(r));
}

void sharpen::PosixFileChannel::DoRead(char *buf,
                                       std::size_t bufSize,
                                       std::uint64_t offset,
                                       sharpen::Future<std::size_t> *future) {
#if (defined SHARPEN_HAS_IOURING) && !(defined SHARPEN_FORCE_NORMAL_FILE_IO)
    assert(this->queue_);
    auto *st = this->InitStruct(buf, bufSize, future);
    if (!st) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe, 0, sizeof(sqe));
    sqe.opcode = IORING_OP_READV;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    sqe.addr = reinterpret_cast<std::uint64_t>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalRead(buf, bufSize, offset, future);
#endif
}


void sharpen::PosixFileChannel::DoAllocate(std::uint64_t offset,
                                           std::size_t size,
                                           sharpen::Future<std::size_t> *future) {
#if (defined SHARPEN_HAS_IOURING) && !(defined SHARPEN_FORCE_NORMAL_FILE_IO)
    assert(this->queue_);
    auto *st = this->InitStruct(nullptr, 0, future);
    if (!st) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe, 0, sizeof(sqe));
    sqe.opcode = IORING_OP_FALLOCATE;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Allocate);
    st->vec_.iov_len = size;
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    // addr should be size of fallocate
    sqe.addr = static_cast<std::uint64_t>(size);
    // len should be flags of fallocate
    sqe.len = FALLOC_FL_KEEP_SIZE;
    sqe.off = offset;
    sqe.fd = this->handle_;
    
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalAllocate(offset, size, future);
#endif
}

void sharpen::PosixFileChannel::DoDeallocate(std::uint64_t offset,
                                             std::size_t size,
                                             sharpen::Future<std::size_t> *future) {
#if (defined SHARPEN_HAS_IOURING) && !(defined SHARPEN_FORCE_NORMAL_FILE_IO)
    assert(this->queue_);
    auto *st = this->InitStruct(nullptr, 0, future);
    if (!st) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe, 0, sizeof(sqe));
    sqe.opcode = IORING_OP_FALLOCATE;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Deallocate);
    st->vec_.iov_len = size;
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    // addr should be size of fallocate
    sqe.addr = static_cast<std::uint64_t>(size);
    // len should be flags of fallocate
    sqe.len = FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalDeallocate(offset, size, future);
#endif
}

void sharpen::PosixFileChannel::WriteAsync(const char *buf,
                                           std::size_t bufSize,
                                           std::uint64_t offset,
                                           sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::DoWrite, this, buf, bufSize, offset, &future));
}

void sharpen::PosixFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,
                                           std::size_t bufferOffset,
                                           std::uint64_t offset,
                                           sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, offset, future);
}

void sharpen::PosixFileChannel::ReadAsync(char *buf,
                                          std::size_t bufSize,
                                          std::uint64_t offset,
                                          sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::DoRead, this, buf, bufSize, offset, &future));
}

void sharpen::PosixFileChannel::ReadAsync(sharpen::ByteBuffer &buf,
                                          std::size_t bufferOffset,
                                          std::uint64_t offset,
                                          sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, offset, future);
}

void sharpen::PosixFileChannel::OnEvent(sharpen::IoEvent *event) {
#ifdef SHARPEN_HAS_IOURING
    assert(this->queue_);
    std::unique_ptr<sharpen::IoUringStruct> st{
        reinterpret_cast<sharpen::IoUringStruct *>(event->GetData())};
    if (event->IsReadEvent() || event->IsWriteEvent()) {
        sharpen::Future<std::size_t> *future =
            reinterpret_cast<sharpen::Future<std::size_t> *>(st->data_);
        if (event->IsErrorEvent()) {
            future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
            return;
        }
        future->Complete(st->length_);
        return;
    } else if (event->IsAllocateEvent() || event->IsDeallocateEvent()) {
        sharpen::Future<std::size_t> *future =
            reinterpret_cast<sharpen::Future<std::size_t> *>(st->data_);
        if (event->IsErrorEvent()) {
            future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
            return;
        }
        future->Complete(st->vec_.iov_len);
        return;
    }
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void> *>(st->data_);
    if (event->IsErrorEvent()) {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete();
#else
    // do nothing
    (void)event;
#endif
}

std::uint64_t sharpen::PosixFileChannel::GetFileSize() const {
    struct stat buf;
    int r = ::fstat(this->handle_, &buf);
    if (r == -1) {
        sharpen::ThrowLastError();
    }
    return buf.st_size;
}

void sharpen::PosixFileChannel::Register(sharpen::EventLoop &loop) {
    this->loop_ = &loop;
#ifdef SHARPEN_HAS_IOURING
    sharpen::EpollSelector *selector =
        reinterpret_cast<sharpen::EpollSelector *>(this->loop_->GetSelectorPtr());
    this->queue_ = selector->GetIoUring();
#endif
}

sharpen::FileMemory sharpen::PosixFileChannel::MapMemory(std::size_t size, std::uint64_t offset) {
    void *addr = ::mmap64(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, this->handle_, offset);
    if (addr == MAP_FAILED) {
        sharpen::ThrowLastError();
    }
    return {addr, size};
}

void sharpen::PosixFileChannel::Truncate() {
    if (::ftruncate(this->handle_, 0) == -1) {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Truncate(std::uint64_t size) {
    if (::ftruncate64(this->handle_, size) == -1) {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Flush() {
    // if we use sync write flag
    // skip flush system call
    if (this->syncWrite_) {
        return;
    }
    if (::fsync(this->handle_) == -1) {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::NormalFlush(sharpen::Future<void> *future) {
    if (::fsync(this->handle_) == -1) {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete();
}

void sharpen::PosixFileChannel::DoFlush(sharpen::Future<void> *future) {
    assert(future != nullptr);
#if (defined SHARPEN_HAS_IOURING) && !(defined SHARPEN_FORCE_NORMAL_FILE_IO)
    assert(this->queue_);
    auto *st = this->InitStruct(future);
    if (!st) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe, 0, sizeof(sqe));
    sqe.opcode = IORING_OP_FSYNC;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Flush);
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalFlush(future);
#endif
}

void sharpen::PosixFileChannel::FlushAsync(sharpen::Future<void> &future) {
    // if we use sync write flag
    // skip flush system call
    if (this->syncWrite_) {
        future.Complete();
        return;
    }
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::DoFlush, this, &future));
}

void sharpen::PosixFileChannel::AllocateAsync(sharpen::Future<std::size_t> &future,
                                              std::uint64_t offset,
                                              std::size_t size) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::DoAllocate,this,offset,size,&future));
}

void sharpen::PosixFileChannel::DeallocateAsync(sharpen::Future<std::size_t> &future,
                                                std::uint64_t offset,
                                                std::size_t size) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::DoDeallocate,this,offset,size,&future));
}

std::size_t sharpen::PosixFileChannel::GetPath(char *path, std::size_t size) const {
    thread_local char buf[sharpen::GetMaxPath() + 1] = {0};
    std::memset(buf,0,sizeof(buf));
    snprintf(buf,sizeof(buf), "/proc/self/fd/%d", this->handle_);
    ssize_t r{readlink(buf,path,size)};
    while(r == -1 && sharpen::GetLastError() == EINTR) {
        r = readlink(buf,path,size);
    }
    if(r == -1) {
        sharpen::ThrowLastError();
    }
    return r;
}

#endif