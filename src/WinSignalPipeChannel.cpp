#include <sharpen/WinPipeSignalChannel.hpp>

#ifdef SHARPEN_HAS_WINSIGNALPIEPECHANNEL

#include <cstdio>
#include <cassert>

#include <sharpen/EventLoop.hpp>

sharpen::WinPipeSignalChannel::WinPipeSignalChannel(sharpen::FileHandle reader,sharpen::FileHandle writer,sharpen::SignalMap &map)
    :Base()
    ,writer_()
    ,map_(&map)
{
    this->handle_ = reader;
    this->writer_ = writer;
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    assert(this->writer_ != INVALID_HANDLE_VALUE);
    //register closer
    using FnPtr = void(*)(sharpen::FileHandle,sharpen::FileHandle,sharpen::SignalMap *);
    FnPtr doClosePtr{static_cast<FnPtr>(&Self::DoClose)};
    this->closer_ = std::bind(doClosePtr,std::placeholders::_1,this->GetWriter(),this->map_);
}

void sharpen::WinPipeSignalChannel::DoClose(sharpen::FileHandle handle,sharpen::FileHandle writer,sharpen::SignalMap *map) noexcept
{
    assert(map != nullptr);
    map->Unregister(writer);
    sharpen::CloseFileHandle(writer);
    sharpen::CloseFileHandle(handle);
}

sharpen::FileHandle sharpen::WinPipeSignalChannel::GetReader() const noexcept
{
    return this->handle_;
}

sharpen::FileHandle sharpen::WinPipeSignalChannel::GetWriter() const noexcept
{
    return this->writer_;
}

void sharpen::WinPipeSignalChannel::InitOverlapped(OVERLAPPED &ol)
{
    std::memset(&ol,0,sizeof(ol));
}

void sharpen::WinPipeSignalChannel::InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct)
{
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.length_ = 0;
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinPipeSignalChannel::OnEvent(sharpen::IoEvent *event)
{
    assert(event != nullptr);
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(reinterpret_cast<sharpen::IocpOverlappedStruct *>(event->GetData()));
    sharpen::Future<std::size_t> *future = reinterpret_cast<sharpen::Future<std::size_t>*>(ev->data_);
    if(event->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete(ev->length_);
}

void sharpen::WinPipeSignalChannel::RequestRead(char *sigs,std::size_t size,sharpen::Future<std::size_t> *future)
{
    assert(future != nullptr);
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
    if(!olStruct)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    //record future
    olStruct->data_ = future;
    BOOL r = ::ReadFile(this->handle_,sigs,static_cast<DWORD>(size),nullptr,&olStruct->ol_);
    if(r == FALSE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if(err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinPipeSignalChannel::ReadAsync(sharpen::SignalBuffer &signals,sharpen::Future<std::size_t> &future)
{
    assert(signals.Data() != nullptr || (signals.Data() == nullptr && signals.GetSize() == 0));
    if(!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::RequestRead,this,signals.Data(),signals.GetSize(),&future));
}

#endif