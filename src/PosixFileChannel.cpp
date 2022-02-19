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

void sharpen::PosixFileChannel::NormalWrite(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future)
{
    ssize_t r = ::pwrite64(this->handle_,buf,bufSize,offset);
    if(r == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(r);
}

#ifdef SHARPEN_HAS_IOURING

sharpen::IoUringStruct *sharpen::PosixFileChannel::InitStruct(void *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
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

void sharpen::PosixFileChannel::DoWrite(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future)
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
    sqe.user_data = reinterpret_cast<sharpen::Uint64>(st);
    sqe.addr = reinterpret_cast<sharpen::Uint64>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalWrite(buf,bufSize,offset,future);
#endif
}

void sharpen::PosixFileChannel::NormalRead(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future)
{
    ssize_t r = ::pread64(this->handle_,buf,bufSize,offset);
    if(r == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(r);
}

void sharpen::PosixFileChannel::DoRead(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> *future)
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
    sqe.user_data = reinterpret_cast<sharpen::Uint64>(st);
    sqe.addr = reinterpret_cast<sharpen::Uint64>(&st->vec_);
    sqe.len = 1;
    sqe.off = offset;
    sqe.fd = this->handle_;
    this->queue_->SubmitIoRequest(sqe);
#else
    this->NormalRead(buf,bufSize,offset,future);
#endif
}

void sharpen::PosixFileChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::DoWrite,this,buf,bufSize,offset,&future));
}
        
void sharpen::PosixFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::PosixFileChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::DoRead,this,buf,bufSize,offset,&future));
}
        
void sharpen::PosixFileChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
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
    sharpen::Future<sharpen::Size> *future = reinterpret_cast<sharpen::Future<sharpen::Size>*>(st->data_);
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

sharpen::Uint64 sharpen::PosixFileChannel::GetFileSize() const
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

sharpen::FileMemory sharpen::PosixFileChannel::MapMemory(sharpen::Size size,sharpen::Uint64 offset)
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

void sharpen::PosixFileChannel::Truncate(sharpen::Uint64 size)
{
    if(::ftruncate64(this->handle_,size) == -1)
    {
        sharpen::ThrowLastError();
    }
}

#endif