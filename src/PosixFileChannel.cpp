#include <sharpen/PosixFileChannel.hpp>

#ifdef SHARPEN_HAS_POSIXFILE

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <cassert>

#include <sharpen/SystemError.hpp>
#include <sharpen/EventLoop.hpp>

#ifdef SHARPEN_HAS_IOURING
#include <new>
#include <cstring>

#include <sharpen/EpollSelector.hpp>
#endif

sharpen::PosixFileChannel::PosixFileChannel(sharpen::FileHandle handle)
    :MyBase()
#ifdef SHARPEN_HAS_IOURING
    ,queue_(nullptr)
#endif
{
    assert(handle != -1);
    this->handle_ = handle;
}

void sharpen::PosixFileChannel::NormalWrite(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
    ssize_t r;
    do
    {
        r = ::pwrite64(this->handle_,buf,bufSize,offset);
    } while (r == -1 && sharpen::GetLastError() == EINTR);
    if(r == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(static_cast<std::size_t>(r));
}

#ifdef SHARPEN_HAS_IOURING

sharpen::IoUringStruct *sharpen::PosixFileChannel::InitStruct(void *buf,std::size_t bufSize,sharpen::Future<std::size_t> *future)
{
    sharpen::IoUringStruct *st = new (std::nothrow) sharpen::IoUringStruct();
    if(!st)
    {
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
#endif

void sharpen::PosixFileChannel::DoWrite(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
#ifdef SHARPEN_HAS_IOURING
    if(!this->queue_)
    {
        this->NormalWrite(buf,bufSize,offset,future);
        return;
    }
    auto *st = this->InitStruct(const_cast<char*>(buf),bufSize,future);
    if(!st)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe,0,sizeof(sqe));
    sqe.opcode = IORING_OP_WRITEV;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Write);
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    sqe.addr = reinterpret_cast<std::uint64_t>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalWrite(buf,bufSize,offset,future);
#endif
}

void sharpen::PosixFileChannel::NormalRead(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
    ssize_t r;
    do
    {
        r = ::pread64(this->handle_,buf,bufSize,offset);
    } while (r == -1 && sharpen::GetLastError() == EINTR);
    if(r == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(static_cast<std::size_t>(r));
}

void sharpen::PosixFileChannel::DoRead(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
#ifdef SHARPEN_HAS_IOURING
    if(!this->queue_)
    {
        this->NormalRead(buf,bufSize,offset,future);
        return;
    }
    auto *st = this->InitStruct(buf,bufSize,future);
    if(!st)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    struct io_uring_sqe sqe;
    std::memset(&sqe,0,sizeof(sqe));
    sqe.opcode = IORING_OP_READV;
    st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    sqe.user_data = reinterpret_cast<std::uint64_t>(st);
    sqe.addr = reinterpret_cast<std::uint64_t>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalRead(buf,bufSize,offset,future);
#endif
}

void sharpen::PosixFileChannel::WriteAsync(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::DoWrite,this,buf,bufSize,offset,&future));
}
        
void sharpen::PosixFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::PosixFileChannel::ReadAsync(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::DoRead,this,buf,bufSize,offset,&future));
}
        
void sharpen::PosixFileChannel::ReadAsync(sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::PosixFileChannel::OnEvent(sharpen::IoEvent *event)
{
#ifdef SHARPEN_HAS_IOURING
    assert(this->queue_);
    std::unique_ptr<sharpen::IoUringStruct> st{reinterpret_cast<sharpen::IoUringStruct*>(event->GetData())};
    sharpen::Future<std::size_t> *future = reinterpret_cast<sharpen::Future<std::size_t>*>(st->data_);
    if(event->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete(st->length_);
#else
    //do nothing
    (void)event;
#endif
}

std::uint64_t sharpen::PosixFileChannel::GetFileSize() const
{
    struct stat buf;
    int r = ::fstat(this->handle_,&buf);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
    return buf.st_size;
}

void sharpen::PosixFileChannel::Register(sharpen::EventLoop *loop)
{
    this->loop_ = loop;
#ifdef SHARPEN_HAS_IOURING
    sharpen::EpollSelector *selector = reinterpret_cast<sharpen::EpollSelector*>(this->loop_->GetSelectorPtr());
    this->queue_ = selector->GetRing();
#endif
}

sharpen::FileMemory sharpen::PosixFileChannel::MapMemory(std::size_t size,std::uint64_t offset)
{
    void *addr = ::mmap64(nullptr,size,PROT_READ|PROT_WRITE,MAP_SHARED,this->handle_,offset);
    if(addr == MAP_FAILED)
    {
        sharpen::ThrowLastError();
    }
    return {addr,size};
}

void sharpen::PosixFileChannel::Truncate()
{
    if(::ftruncate(this->handle_,0) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Truncate(std::uint64_t size)
{
    if(::ftruncate64(this->handle_,size) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Flush()
{
    if(::fsync(this->handle_) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Allocate(std::uint64_t offset,std::size_t size)
{
    if(::fallocate64(this->handle_,FALLOC_FL_KEEP_SIZE,offset,size) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::PosixFileChannel::Deallocate(std::uint64_t offset,std::size_t size)
{
    if(::fallocate64(this->handle_,FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE,offset,size) == -1)
    {
        sharpen::ThrowLastError();
    }
}

#endif