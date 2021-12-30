#include <sharpen/WinFileChannel.hpp>

#ifdef SHARPEN_HAS_WINFILE

#include <cstring>
#include <cassert>

#include <Windows.h>
#include <io.h>

sharpen::WinFileChannel::WinFileChannel(sharpen::FileHandle handle)
    :Mybase()
{
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
}

void sharpen::WinFileChannel::InitOverlapped(OVERLAPPED &ol,sharpen::Uint64 offset)
{
    std::memset(&ol,0,sizeof(ol));
    LARGE_INTEGER off;
    off.QuadPart = offset;
    ol.Offset = off.LowPart;
    ol.OffsetHigh = off.HighPart;
}

void sharpen::WinFileChannel::InitOverlappedStruct(IocpOverlappedStruct &olStruct,sharpen::Uint64 offset)
{
    //init overlapped
    sharpen::WinFileChannel::InitOverlapped(olStruct.ol_,offset);
    //init length
    olStruct.length_ = 0;
    //set olStruct data
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinFileChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    IocpOverlappedStruct *olStruct = new (std::nothrow) IocpOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,offset);
    olStruct->event_.SetData(olStruct);
    //record future
    olStruct->data_ = &future;
    //request
    BOOL r = ::WriteFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&olStruct->ol_);
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            future.Fail(sharpen::MakeLastErrorPtr());
        }
    }
}
        
void sharpen::WinFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::WinFileChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,offset);
    olStruct->event_.SetData(olStruct);
    //record future
    olStruct->data_ = &future;
    BOOL r = ::ReadFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&olStruct->ol_);
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
        
void sharpen::WinFileChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::WinFileChannel::OnEvent(sharpen::IoEvent *olStruct)
{
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(reinterpret_cast<sharpen::IocpOverlappedStruct*>(olStruct->GetData()));
    MyFuturePtr future = reinterpret_cast<MyFuturePtr>(ev->data_);
    if (olStruct->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct->GetErrorCode()));
        return;
    }
    future->Complete(ev->length_);
}

sharpen::Uint64 sharpen::WinFileChannel::GetFileSize() const
{
    LARGE_INTEGER li;
    BOOL r = ::GetFileSizeEx(this->handle_,&li);
    if (r == FALSE)
    {
        sharpen::ThrowLastError();
    }
    return li.QuadPart;
}

sharpen::FileMemory sharpen::WinFileChannel::MapMemory(sharpen::Size size,sharpen::Uint64 offset)
{
    LARGE_INTEGER li;
    li.QuadPart = offset + size;
    HANDLE mapObject = ::CreateFileMappingA(this->handle_,nullptr,PAGE_READWRITE,li.HighPart,li.LowPart,nullptr);
    if(!mapObject)
    {
        sharpen::ThrowLastError();
    }
    li.QuadPart = offset;
    void *addr = ::MapViewOfFile(mapObject,FILE_MAP_ALL_ACCESS,li.LowPart,li.HighPart,size);
    ::CloseHandle(mapObject);
    if(addr == nullptr)
    {
        sharpen::ThrowLastError();
    }
    return {this->handle_,addr,size};
}

#endif