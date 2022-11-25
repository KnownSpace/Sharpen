#include <sharpen/PosixOutputPipeChannel.hpp>
#ifdef SHARPEN_HAS_POSIXOUTPUTPIPE

#include <stdexcept>
#include <cassert>

#include <sharpen/EventLoop.hpp>

sharpen::PosixOutputPipeChannel::PosixOutputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
    ,writer_()
    ,writeable_(false)
{
    assert(handle != -1);
    this->handle_ = handle;
}

sharpen::PosixOutputPipeChannel::~PosixOutputPipeChannel() noexcept
{
    this->writer_.CancelAllIo(ECANCELED);
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

void sharpen::PosixOutputPipeChannel::TryWrite(const char *buf,std::size_t bufSize,Callback cb)
{
    this->writer_.AddPendingTask(const_cast<char*>(buf),bufSize,std::move(cb));
    if (this->writeable_)
    {
        this->DoWrite();
    }
}

void sharpen::PosixOutputPipeChannel::RequestWrite(const char *buf,std::size_t bufSize,sharpen::Future<std::size_t> *future)
{
    using FnPtr = void(*)(sharpen::EventLoop *,sharpen::Future<std::size_t>*,ssize_t);
    Callback cb = std::bind(static_cast<FnPtr>(&sharpen::PosixOutputPipeChannel::CompleteWriteCallback),this->loop_,future,std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&sharpen::PosixOutputPipeChannel::TryWrite,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixOutputPipeChannel::WriteAsync(const char *buf,std::size_t bufSize,sharpen::Future<std::size_t> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestWrite(buf,bufSize,&future);
}

void sharpen::PosixOutputPipeChannel::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset,sharpen::Future<std::size_t> &future)
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

void sharpen::PosixOutputPipeChannel::CompleteWriteCallback(sharpen::EventLoop *loop,sharpen::Future<std::size_t> *future,ssize_t size) noexcept
{
    if(size == -1)
    {
        loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::Fail,future,sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::CompleteForBind,future,static_cast<std::size_t>(size)));
}
#endif