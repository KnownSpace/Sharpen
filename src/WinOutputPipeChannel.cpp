#include <sharpen/WinOutputPipeChannel.hpp>
#ifdef SHARPEN_HAS_WINOUTPUTPIPE

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <new>

sharpen::WinOutputPipeChannel::WinOutputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
{
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
}

sharpen::WinOutputPipeChannel::~WinOutputPipeChannel() noexcept
{
    if (this->handle_ != INVALID_HANDLE_VALUE)
    {
        ::CancelIoEx(this->handle_,nullptr);
    }
}

void sharpen::WinOutputPipeChannel::InitOverlapped(OVERLAPPED &ol)
{
    std::memset(&ol,0,sizeof(ol));
}

void sharpen::WinOutputPipeChannel::InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct)
{
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.length_ = 0;
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinOutputPipeChannel::RequestWrite(const char *buf,std::size_t bufSize,sharpen::Future<std::size_t> *future)
{
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
    if (!olStruct)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    //record future
    olStruct->data_ = future;
    BOOL r = ::WriteFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&(olStruct->ol_));
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinOutputPipeChannel::WriteAsync(const char *buf,std::size_t bufSize,sharpen::Future<std::size_t> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::RequestWrite,this,buf,bufSize,&future));
}

void sharpen::WinOutputPipeChannel::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufOffset,sharpen::Future<std::size_t> &future)
{
    if (bufOffset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufOffset,buf.GetSize() - bufOffset,future);
}

void sharpen::WinOutputPipeChannel::OnEvent(sharpen::IoEvent *event)
{
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(reinterpret_cast<sharpen::IocpOverlappedStruct*>(event->GetData()));
    sharpen::Future<std::size_t> *future = reinterpret_cast<sharpen::Future<std::size_t>*>(ev->data_);
    if (event->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete(ev->length_);
}
#endif