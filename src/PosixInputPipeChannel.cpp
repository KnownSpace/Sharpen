#include <sharpen/PosixInputPipeChannel.hpp>
#ifdef SHARPEN_HAS_POSIXINPUTPIPE

#include <cassert>
#include <stdexcept>

sharpen::PosixInputPipeChannel::PosixInputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
    ,reader_()
    ,readable_(false)
{
    assert(handle != -1);
    this->handle_ = handle;
}

void sharpen::PosixInputPipeChannel::HandleRead()
{
    this->DoRead();
}

void sharpen::PosixInputPipeChannel::DoRead()
{
    bool executed;
    bool blocking;
    this->reader_.Execute(this->handle_,executed,blocking);
    this->readable_ = !executed || !blocking;
}

void sharpen::PosixInputPipeChannel::TryRead(char *buf,sharpen::Size bufSize,Callback cb)
{
    this->reader_.AddPendingTask(buf,bufSize,std::move(cb));
    if(this->readable_)
    {
        this->DoRead();
    }
}

void sharpen::PosixInputPipeChannel::RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    using FnPtr = void(*)(sharpen::EventLoop *,sharpen::Future<sharpen::Size> *,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixInputPipeChannel::CompleteReadCallback),this->loop_,future,std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&sharpen::PosixInputPipeChannel::TryRead,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixInputPipeChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestRead(buf,bufSize,&future);
}

void sharpen::PosixInputPipeChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufOffset,sharpen::Future<sharpen::Size> &future)
{
    if(bufOffset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufOffset,buf.GetSize() - bufOffset,future);
}

void sharpen::PosixInputPipeChannel::OnEvent(sharpen::IoEvent *event)
{
    if (event->IsReadEvent() || event->IsErrorEvent() || event->IsCloseEvent())
    {
        this->HandleRead();
    }
}

void sharpen::PosixInputPipeChannel::CompleteReadCallback(sharpen::EventLoop *loop,sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept
{
    if(size == -1)
    {
        loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::Size>::Fail,future,sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<sharpen::Size>::CompleteForBind,future,static_cast<sharpen::Size>(size)));
}
#endif