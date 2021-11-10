#include <sharpen/PosixOutputPipeChannel.hpp>
#ifdef SHARPEN_HAS_POSIXOUTPUTPIPE

#include <stdexcept>
#include <cassert>

sharpen::PosixOutputPipeChannel::PosixOutputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
    ,writer_()
    ,writeable_(false)
{
    assert(handle != -1);
    this->handle_ = handle;
}

void sharpen::PosixOutputPipeChannel::DoWrite()
{
    bool executed;
    bool blocking;
    this->writer_.Execute(this->handle_,executed,blocking);
    this->writeable_ = !executed || !blocking;
}

void sharpen::PosixOutputPipeChannel::HandleWrite()
{
    this->DoWrite();
}

void sharpen::PosixOutputPipeChannel::TryWrite(const char *buf,sharpen::Size bufSize,Callback cb)
{
    this->writer_.AddPendingTask(const_cast<char*>(buf),bufSize,std::move(cb));
    if (this->writeable_)
    {
        this->DoWrite();
    }
}

void sharpen::PosixOutputPipeChannel::RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    using FnPtr = void(*)(sharpen::EventLoop *,sharpen::Future<sharpen::Size>*,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixOutputPipeChannel::CompleteWriteCallback),this->loop_,future,std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&sharpen::PosixOutputPipeChannel::TryWrite,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixOutputPipeChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestWrite(buf,bufSize,&future);
}

void sharpen::PosixOutputPipeChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future)
{
    if (bufferOffset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,future);
}

void sharpen::PosixOutputPipeChannel::OnEvent(sharpen::IoEvent *event)
{
    if (event->IsWriteEvent() || event->IsCloseEvent() || event->IsErrorEvent())
    {
        this->HandleWrite();
    }
}

void sharpen::PosixOutputPipeChannel::CompleteWriteCallback(sharpen::EventLoop *loop,sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept
{
    if(size == -1)
    {
        loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::Size>::Fail,future,sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::Size>::CompleteForBind,future,static_cast<sharpen::Size>(size)));
}
#endif