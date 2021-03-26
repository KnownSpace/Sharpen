#include <sharpen/PosixFileChannel.hpp>

#ifdef SHARPEN_HAS_POSIXFILE

#include <sys/stat.h>

#include <cassert>

#include <sharpen/SystemError.hpp>

sharpen::PosixFileChannel::PosixFileChannel(sharpen::FileHandle handle)
    :MyBase()
{
    assert(handle != -1);
    this->handle_ = handle;
}

void sharpen::PosixFileChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::FileHandle fd = this->handle_;
    this->loop_.QueueInLoop([buf,bufSize,offset,&future,fd]() mutable
    {
        ssize_t r = ::pwrite(fd,buf,bufSize,offset);
        if (r < 0)
        {
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
        future.Complete(r);
    });
}
        
void sharpen::PosixFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    this->WriteAsync(buf.Data(),buf.GetSize(),offset,future);
}

void sharpen::PosixFileChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::FileHandle fd = this->handle_;
    this->loop_.QueueInLoop([buf,bufSize,offset,&future,fd]() mutable
    {
        ssize_t r = ::pread(fd,buf,bufSize,offset);
        if (r < 0)
        {
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
        future.Complete(r);
    });
}
        
void sharpen::PosixFileChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    this->ReadAsync(buf.Data(),buf.GetSize(),offset,future);
}

void sharpen::PosixFileChannel::OnEvent(sharpen::IoEvent *event)
{
    //do nothing
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

#endif