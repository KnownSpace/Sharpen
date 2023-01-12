#include <sharpen/WinFileChannel.hpp>

#ifdef SHARPEN_HAS_WINFILE

#include <cstring>
#include <cassert>

#include <Windows.h>
#include <io.h>
#include <winioctl.h>

#include <sharpen/EventLoop.hpp>

sharpen::Optional<bool> sharpen::WinFileChannel::supportSparseFile_{sharpen::EmptyOpt};

sharpen::WinFileChannel::WinFileChannel(sharpen::FileHandle handle)
    :Mybase()
    ,sparesFile_(false)
{
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
}

void sharpen::WinFileChannel::InitOverlapped(OVERLAPPED &ol,std::uint64_t offset)
{
    std::memset(&ol,0,sizeof(ol));
    LARGE_INTEGER off;
    off.QuadPart = offset;
    ol.Offset = off.LowPart;
    ol.OffsetHigh = off.HighPart;
}

void sharpen::WinFileChannel::InitOverlappedStruct(IocpOverlappedStruct &olStruct,std::uint64_t offset)
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

void sharpen::WinFileChannel::RequestWrite(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
    IocpOverlappedStruct *olStruct = new (std::nothrow) IocpOverlappedStruct();
    if (!olStruct)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,offset);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Write);
    //record future
    olStruct->data_ = future;
    //request
    BOOL r = ::WriteFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&olStruct->ol_);
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            if(err == ERROR_HANDLE_EOF || err == sharpen::ErrorCancel)
            {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
        }
    }
}

void sharpen::WinFileChannel::WriteAsync(const char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoopSoon(std::bind(&Self::RequestWrite,this,buf,bufSize,offset,&future));
}
        
void sharpen::WinFileChannel::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::WinFileChannel::RequestRead(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> *future)
{
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
    if (!olStruct)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,offset);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    //record future
    olStruct->data_ = future;
    BOOL r = ::ReadFile(this->handle_,buf,static_cast<DWORD>(bufSize),nullptr,&olStruct->ol_);
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            if(err == ERROR_HANDLE_EOF || err == sharpen::ErrorCancel)
            {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinFileChannel::ReadAsync(char *buf,std::size_t bufSize,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->loop_->RunInLoop(std::bind(&Self::RequestRead,this,buf,bufSize,offset,&future));
}
        
void sharpen::WinFileChannel::ReadAsync(sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset,sharpen::Future<std::size_t> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,offset,future);
}

void sharpen::WinFileChannel::OnEvent(sharpen::IoEvent *event)
{
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(reinterpret_cast<sharpen::IocpOverlappedStruct*>(event->GetData()));
    MyFuturePtr future = reinterpret_cast<MyFuturePtr>(ev->data_);
    if (event->IsErrorEvent())
    {
        sharpen::ErrorCode code{event->GetErrorCode()};
        if(code == ERROR_HANDLE_EOF || code == sharpen::ErrorCancel)
        {
            future->Complete(static_cast<std::size_t>(0));
            return;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    future->Complete(ev->length_);
}

std::uint64_t sharpen::WinFileChannel::GetFileSize() const
{
    LARGE_INTEGER li;
    BOOL r = ::GetFileSizeEx(this->handle_,&li);
    if (r == FALSE)
    {
        sharpen::ThrowLastError();
    }
    return li.QuadPart;
}

sharpen::FileMemory sharpen::WinFileChannel::MapMemory(std::size_t size,std::uint64_t offset)
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

void sharpen::WinFileChannel::Truncate()
{
    if(::SetFilePointer(this->handle_,0,nullptr,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        sharpen::ThrowLastError();
    }
    if(::SetEndOfFile(this->handle_) == FALSE)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::WinFileChannel::Truncate(std::uint64_t size)
{
    LARGE_INTEGER li,old;
    li.QuadPart = size;
    if(::SetFilePointerEx(this->handle_,li,&old,FILE_BEGIN) == FALSE)
    {
        sharpen::ThrowLastError();
    }
    if(::SetEndOfFile(this->handle_) == FALSE)
    {
        sharpen::ThrowLastError();
    }
    ::SetFilePointerEx(this->handle_,old,nullptr,FILE_BEGIN);
}

void sharpen::WinFileChannel::Flush()
{
    if (::FlushFileBuffers(this->handle_) == FALSE)
    {
        sharpen::ThrowLastError();
    }
}

bool sharpen::WinFileChannel::SupportSparseFile(const char *rootName) noexcept
{
    DWORD flag{0};
    if(::GetVolumeInformationA(rootName,nullptr,0,nullptr,nullptr,&flag,nullptr,0) == FALSE)
    {
        return false;
    }
    return flag|FILE_SUPPORTS_SPARSE_FILES;
}

void sharpen::WinFileChannel::EnableSparesFile()
{
    if(!Self::supportSparseFile_.Exist())
    {
        //FIXME:use this file's root
        Self::supportSparseFile_.Construct(Self::SupportSparseFile(nullptr));
    }
    if(!Self::supportSparseFile_.Get())
    {
        sharpen::ThrowSystemError(sharpen::ErrorOperationNotSupport);
    }
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct{};
    if(!olStruct)
    {
        throw std::bad_alloc{};
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,0);
    olStruct->event_.SetData(olStruct);
    //record future
    sharpen::AwaitableFuture<std::size_t> future;
    olStruct->data_ = &future;
    if(::DeviceIoControl(this->handle_,FSCTL_SET_SPARSE,nullptr,0,nullptr,0,nullptr,&olStruct->ol_) == FALSE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            sharpen::ThrowLastError();
        }
    }
    //blocking
    future.Await();
}

void sharpen::WinFileChannel::Allocate(std::uint64_t offset,std::size_t size)
{
    (void)offset;
    FILE_ALLOCATION_INFO alloc;
    alloc.AllocationSize.QuadPart = this->GetFileSize() + size;
    if(::SetFileInformationByHandle(this->handle_,FILE_INFO_BY_HANDLE_CLASS::FileAllocationInfo,&alloc,sizeof(alloc)) == FALSE)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::WinFileChannel::Deallocate(std::uint64_t offset,std::size_t size)
{
    if(!this->sparesFile_)
    {
        this->EnableSparesFile();
        this->sparesFile_ = true;
    }
    FILE_ZERO_DATA_INFORMATION zero;
    zero.FileOffset.QuadPart = offset;
    zero.BeyondFinalZero.QuadPart = offset + size;
    sharpen::IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct{};
    if(!olStruct)
    {
        throw std::bad_alloc{};
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct,0);
    olStruct->event_.SetData(olStruct);
    //record future
    sharpen::AwaitableFuture<std::size_t> future;
    olStruct->data_ = &future;
    if(::DeviceIoControl(this->handle_,FSCTL_SET_ZERO_DATA,&zero,sizeof(zero),nullptr,0,nullptr,&olStruct->ol_) == FALSE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS)
        {
            delete olStruct;
            sharpen::ThrowLastError();
        }
    }
    //blocking
    future.Await();
}

#endif