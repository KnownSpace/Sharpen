#include <sharpen/WinPipeSignalChannel.hpp>

#ifdef SHARPEN_HAS_WINSIGNALPIEPECHANNEL

#include <cstdio>
#include <cassert>

#include <sharpen/EventLoop.hpp>

BOOL CreatePipeEx(OUT LPHANDLE lpReadPipe,OUT LPHANDLE lpWritePipe,IN LPSECURITY_ATTRIBUTES lpPipeAttributes,IN DWORD nSize,DWORD dwReadMode,DWORD dwWriteMode)
{
    static volatile long PipeSerialNumber;

    HANDLE ReadPipeHandle,WritePipeHandle;
    DWORD dwError;
    char PipeNameBuffer[MAX_PATH];
    if((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if(nSize == 0)
    {
        nSize = 4096;
    }
    std::snprintf(PipeNameBuffer,
                  sizeof(PipeNameBuffer),
                  "\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
                  GetCurrentProcessId(),
                  InterlockedIncrement(&PipeSerialNumber)
    );
    ReadPipeHandle = CreateNamedPipeA(
        PipeNameBuffer,
        PIPE_ACCESS_INBOUND | dwReadMode,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,             // Number of pipes
        nSize,         // Out buffer size
        nSize,         // In buffer size
        120 * 1000,    // Timeout in ms
        lpPipeAttributes
    );
    if(!ReadPipeHandle)
    {
        return FALSE;
    }
    WritePipeHandle = CreateFileA(
        PipeNameBuffer,
        GENERIC_WRITE,
        0,                         // No sharing
        lpPipeAttributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | dwWriteMode,
        NULL                       // Template file
    );
    if(INVALID_HANDLE_VALUE == WritePipeHandle)
    {
        dwError = GetLastError();
        CloseHandle(ReadPipeHandle);
        SetLastError(dwError);
        return FALSE;
    }

    *lpReadPipe = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return(TRUE);
}

sharpen::WinPipeSignalChannel::WinPipeSignalChannel(sharpen::FileHandle reader,sharpen::FileHandle writer,sharpen::SignalMap &map)
    :Base()
    ,writer_()
    ,map_(&map)
{
    this->handle_ = reader;
    this->writer_ = writer;
#ifdef SHARPEN_IS_WIN
    assert(this->handle_ != INVALID_HANDLE_VALUE);
    assert(this->writer_ != INVALID_HANDLE_VALUE);
#else
    assert(this->handle_ != -1);
    assert(this->writer_ != -1);
#endif
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

#ifdef SHARPEN_IS_WIN
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
#endif

void sharpen::WinPipeSignalChannel::OnEvent(sharpen::IoEvent *event)
{
    assert(event != nullptr);
#ifdef SHARPEN_IS_WIN
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(reinterpret_cast<sharpen::IocpOverlappedStruct *>(event->GetData()));
    sharpen::Future<std::size_t> *future = reinterpret_cast<sharpen::Future<std::size_t>*>(ev->data_);
    if(event->IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(event->GetErrorCode()));
        return;
    }
    future->Complete(ev->length_);
#endif
}

void sharpen::WinPipeSignalChannel::RequestRead(char *sigs,std::size_t size,sharpen::Future<std::size_t> *future)
{
    assert(future != nullptr);
#ifdef SHARPEN_IS_WIN
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
#else
#endif
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