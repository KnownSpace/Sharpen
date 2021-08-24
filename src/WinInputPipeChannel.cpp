#include <sharpen/WinInputPipeChannel.hpp>
#ifdef SHARPEN_HAS_WININPUTPIPE

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <Windows.h>

sharpen::WinInputPipeChannel::WinInputPipeChannel(sharpen::FileHandle handle)
    :Mybase()
{
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
}

void sharpen::WinInputPipeChannel::InitOverlapped(OVERLAPPED &ol)
{
    std::memset(&ol,0,sizeof(ol));
}

void sharpen::WinInputPipeChannel::InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct)
{
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.length_ = 0;
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinInputPipeChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
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
    BOOL r = ::ReadFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&(olStruct->ol_));
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

void sharpen::WinInputPipeChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufOffset,sharpen::Future<sharpen::Size> &future)
{
    if (bufOffset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufOffset,buf.GetSize() - bufOffset,future);
}

void sharpen::WinInputPipeChannel::OnEvent(sharpen::IoEvent *event)
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