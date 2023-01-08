#include <sharpen/PosixPipeSignalChannel.hpp>

#ifdef SHARPEN_HAS_POSIXPIPESIGNALCHANNEL

#include <sharpen/IoEvent.hpp>
#include <sharpen/EventLoop.hpp>

sharpen::PosixPipeSignalChannel::PosixPipeSignalChannel(sharpen::FileHandle reader,sharpen::FileHandle writer,sharpen::SignalMap &map)
    :Base()
    ,writer_()
    ,map_(&map)
    ,reader_()
    ,readable_(false)
{
    this->handle_ = reader;
    this->writer_ = writer;
    assert(this->handle_ != -1);
    assert(this->writer_ != -1);
    //register closer
    using FnPtr = void(*)(sharpen::FileHandle,sharpen::FileHandle,sharpen::SignalMap *);
    FnPtr doClosePtr{static_cast<FnPtr>(&Self::DoClose)};
    this->closer_ = std::bind(doClosePtr,std::placeholders::_1,this->GetWriter(),this->map_);    
}

sharpen::PosixPipeSignalChannel::~PosixPipeSignalChannel() noexcept
{
    this->reader_.CancelAllIo(sharpen::ErrorCancel);
}

void sharpen::PosixPipeSignalChannel::DoClose(sharpen::FileHandle handle,sharpen::FileHandle writer,sharpen::SignalMap *map) noexcept
{
    map->Unregister(writer);
    sharpen::CloseFileHandle(writer);
    sharpen::CloseFileHandle(handle);
}

sharpen::FileHandle sharpen::PosixPipeSignalChannel::GetReader() const noexcept
{
    return this->handle_;
}

sharpen::FileHandle sharpen::PosixPipeSignalChannel::GetWriter() const noexcept
{
    return this->writer_;
}

void sharpen::PosixPipeSignalChannel::DoRead()
{
    bool executed;
    bool blocking;
    this->reader_.Execute(this->handle_,executed,blocking);
    this->readable_ = !executed || !blocking;
}

void sharpen::PosixPipeSignalChannel::HandleRead()
{
    this->DoRead();
}

void sharpen::PosixPipeSignalChannel::TryRead(char *buf,std::size_t bufSize,Callback cb)
{
    this->reader_.AddPendingTask(buf,bufSize,std::move(cb));
    if(this->readable_)
    {
        this->DoRead();
    }
}

void sharpen::PosixPipeSignalChannel::RequestRead(char *buf,std::size_t bufSize,sharpen::Future<std::size_t> *future)
{
    using FnPtr = void(*)(sharpen::EventLoop *,sharpen::Future<std::size_t> *,ssize_t);
    Callback cb = std::bind(static_cast<FnPtr>(&Self::CompleteReadCallback),this->loop_,future,std::placeholders::_1);
    this->loop_->RunInLoop(std::bind(&Self::TryRead,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixPipeSignalChannel::CompleteReadCallback(sharpen::EventLoop *loop,sharpen::Future<std::size_t> *future,ssize_t size) noexcept
{
    if(size == -1)
    {
        loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::Fail,future,sharpen::MakeLastErrorPtr()));
        return;
    }
    loop->RunInLoopSoon(std::bind(&sharpen::Future<std::size_t>::CompleteForBind,future,static_cast<std::size_t>(size)));
}

void sharpen::PosixPipeSignalChannel::OnEvent(sharpen::IoEvent *event)
{
    if(event->IsReadEvent() || event->IsCloseEvent() || event->IsErrorEvent())
    {
        this->HandleRead();
    }
}

void sharpen::PosixPipeSignalChannel::ReadAsync(sharpen::SignalBuffer &signals,sharpen::Future<std::size_t> &future)
{
    assert(signals.Data() != nullptr || (signals.Data() == nullptr && signals.GetSize() == 0));
    if(!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestRead(signals.Data(),signals.GetSize(),&future);
}
#endif