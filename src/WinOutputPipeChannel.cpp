#include <sharpen/WinOutputPipeChannel.hpp>
#ifdef SHARPEN_HAS_WINOUTPUTPIPE

#include <cassert>
#include <cstring>
#include <stdexcept>

sharpen::WinOutputPipeChannel::WinOutputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
{
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
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
}

void sharpen::WinOutputPipeChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::IocpOverlappedStruct *olStruct = new sharpen::IocpOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    //record future
    olStruct->data_ = &future;
    BOOL r = ::WriteFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&(olStruct->ol_));
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinOutputPipeChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufOffset,sharpen::Future<sharpen::Size> &future)
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
    sharpen::Future<sharpen::Size> *future = reinterpret_cast<sharpen::Future<sharpen::Size>*>(ev->data_);
    if (event->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete(ev->length_);
}
#endif