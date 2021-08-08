#include <sharpen/WinNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_WINSOCKET

#include <cassert>
#include <type_traits>
#include <cstring>

#include <WinSock2.h>
#include <MSWSock.h>


void sharpen::WinNetStreamChannel::InitOverlapped(OVERLAPPED &ol)
{
    std::memset(&ol,0,sizeof(ol));
}

void sharpen::WinNetStreamChannel::InitOverlappedStruct(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::WinNetStreamChannel::InitOverlapped(olStruct.ol_);
    //init buf
    olStruct.buf_.buf = nullptr;
    olStruct.buf_.len = 0;
    //init accepted socket
    olStruct.accepted_ = reinterpret_cast<sharpen::FileHandle>(INVALID_SOCKET);
    //init length
    olStruct.length_ = 0;
    //set olStruct data
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
}

void sharpen::WinNetStreamChannel::Closer(sharpen::FileHandle handle)
{
    SOCKET sock = reinterpret_cast<SOCKET>(handle);
    ::closesocket(sock);
}

sharpen::WinNetStreamChannel::WinNetStreamChannel(sharpen::FileHandle handle,int af)
    :Mybase()
    ,af_(af)
{
    assert(handle != reinterpret_cast<sharpen::FileHandle>(INVALID_SOCKET));
    this->handle_ = handle;
    using FnPtr = void (*) (void);
    this->closer_  = std::move(std::bind(reinterpret_cast<FnPtr>(&sharpen::WinNetStreamChannel::Closer)));
}

void sharpen::WinNetStreamChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Write);
    //record future
    olStruct->data_ = &future;
    //set buf
    olStruct->buf_.buf = const_cast<CHAR*>(buf);
    olStruct->buf_.len = static_cast<ULONG>(bufSize);
    //request
    BOOL r = ::WSASend(reinterpret_cast<SOCKET>(this->handle_),&(olStruct->buf_),1,nullptr,0,reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),nullptr);
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
        
void sharpen::WinNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) 
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,future);
}

void sharpen::WinNetStreamChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) 
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    //record future
    olStruct->data_ = &future;
    //set buf
    olStruct->buf_.buf = buf;
    olStruct->buf_.len = static_cast<ULONG>(bufSize);
    //request
    static DWORD recvFlag = 0;
    BOOL r = ::WSARecv(reinterpret_cast<SOCKET>(this->handle_),&(olStruct->buf_),1,nullptr,&recvFlag,reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),nullptr);
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING&& err != ERROR_SUCCESS)
        {
            delete olStruct;
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) 
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,future);
}

void sharpen::WinNetStreamChannel::OnEvent(sharpen::IoEvent *event)
{
     std::unique_ptr<sharpen::WSAOverlappedStruct> olStruct(reinterpret_cast<sharpen::WSAOverlappedStruct*>(event->GetData()));
     sharpen::IoEvent::EventType ev = olStruct->event_.GetEventType();
     if (ev & sharpen::IoEvent::EventTypeEnum::Accept)
     {
         this->HandleAccept(*olStruct);
     }
     else if (ev & (sharpen::IoEvent::EventTypeEnum::Read | sharpen::IoEvent::EventTypeEnum::Write))
     {
         this->HandleReadAndWrite(*olStruct);
     }
     else if (ev & sharpen::IoEvent::EventTypeEnum::Connect)
     {
         this->HandleConnect(*olStruct);
     }
     else if (ev & sharpen::IoEvent::EventTypeEnum::Sendfile)
     {
         this->HandleSendFile(*olStruct);
     }
     else if (ev & sharpen::IoEvent::EventTypeEnum::Poll)
     {
         this->HandlePoll(*olStruct);
     }
     
}

void sharpen::WinNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Sendfile);
    //record future
    olStruct->data_ = &future;
    //set offset
    LARGE_INTEGER li;
    li.QuadPart = offset;
    olStruct->ol_.Offset = li.LowPart;
    olStruct->ol_.OffsetHigh = li.HighPart;
    //request
    BOOL r = ::TransmitFile(reinterpret_cast<SOCKET>(this->handle_),file->GetHandle(),static_cast<DWORD>(size),0,&(olStruct->ol_),nullptr,0);
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
        
void sharpen::WinNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future)
{
    this->SendFileAsync(file,file->GetFileSize(),0,future);
}

void sharpen::WinNetStreamChannel::AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future)
{
    static LPFN_ACCEPTEX WSAAcceptEx = nullptr;

    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    //get acceptex
    {
        GUID acceptexId = WSAID_ACCEPTEX;
        DWORD dwBytes;
        int iResult = WSAIoctl(reinterpret_cast<SOCKET>(this->handle_), SIO_GET_EXTENSION_FUNCTION_POINTER,&acceptexId, sizeof (acceptexId), &WSAAcceptEx, sizeof (WSAAcceptEx), &dwBytes, NULL, NULL);
        if (iResult != 0)
        {
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Accept);
    //record future
    olStruct->data_ = &future;
    //open client socket
    SOCKET s = ::socket(this->af_, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        delete olStruct;
        future.Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    olStruct->accepted_ = reinterpret_cast<sharpen::FileHandle>(s);
    //alloc buf
    olStruct->buf_.buf = reinterpret_cast<char*>(std::calloc(1024,sizeof(char)));
    if (olStruct->buf_.buf == nullptr)
    {
        delete olStruct;
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //request
    BOOL r = WSAAcceptEx(reinterpret_cast<SOCKET>(this->handle_),reinterpret_cast<SOCKET>(olStruct->accepted_),olStruct->buf_.buf,0,sizeof(SOCKADDR_IN) + 16,sizeof(SOCKADDR_IN) + 16,nullptr,&(olStruct->ol_));
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

void sharpen::WinNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint,sharpen::Future<void> &future)
{
    static LPFN_CONNECTEX WSAConnectEx = nullptr;

    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    //get acceptex
    {
        GUID connectexId = WSAID_CONNECTEX;
        DWORD dwBytes;
        int iResult = WSAIoctl(reinterpret_cast<SOCKET>(this->handle_), SIO_GET_EXTENSION_FUNCTION_POINTER,&connectexId, sizeof (connectexId), &WSAConnectEx, sizeof (WSAConnectEx), &dwBytes, NULL, NULL);
        if (iResult != 0)
        {
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Connect);
    //record future
    olStruct->data_ = &future;
    //request
    BOOL r = WSAConnectEx(reinterpret_cast<SOCKET>(this->handle_),endpoint.GetAddrPtr(),endpoint.GetAddrLen(),nullptr,0,nullptr,&(olStruct->ol_));
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

void sharpen::WinNetStreamChannel::PollReadAsync(sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Poll);
    //record future
    olStruct->data_ = &future;
    //set buf
    olStruct->buf_.buf = nullptr;
    olStruct->buf_.len = 0;
    //request
    static DWORD recvFlag = 0;
    //use WSARecv
    //with buf = nullptr & len = 0
    BOOL r = ::WSARecv(reinterpret_cast<SOCKET>(this->handle_),&(olStruct->buf_),1,nullptr,&recvFlag,reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),nullptr);
    if (r != TRUE)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING&& err != ERROR_SUCCESS)
        {
            delete olStruct;
            future.Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::PollWriteAsync(sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new sharpen::WSAOverlappedStruct();
    if (!olStruct)
    {
        future.Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    //init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Poll);
    //record future
    olStruct->data_ = &future;
    //set buf
    olStruct->buf_.buf = nullptr;
    olStruct->buf_.len = 0;
    //request
    //use WSASend
    //with buf = nullptr & len = 0
    BOOL r = ::WSASend(reinterpret_cast<SOCKET>(this->handle_),&(olStruct->buf_),1,nullptr,0,reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),nullptr);
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

void sharpen::WinNetStreamChannel::HandleReadAndWrite(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::Future<sharpen::Size> *future = reinterpret_cast<sharpen::Future<sharpen::Size>*>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct.event_.GetErrorCode()));
        return;
    }
    future->Complete(olStruct.length_);
}

void sharpen::WinNetStreamChannel::HandleAccept(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::Future<sharpen::NetStreamChannelPtr> *future = reinterpret_cast<sharpen::Future<sharpen::NetStreamChannelPtr>*>(olStruct.data_);
    std::free(olStruct.buf_.buf);
    if (olStruct.event_.IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct.event_.GetErrorCode()));
        return;
    }
    ::setsockopt(reinterpret_cast<SOCKET>(olStruct.accepted_),SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,reinterpret_cast<char*>(&this->handle_),sizeof(SOCKET));
    sharpen::NetStreamChannelPtr channel = std::make_shared<sharpen::WinNetStreamChannel>(olStruct.accepted_,this->af_);
    future->Complete(channel);
}

void sharpen::WinNetStreamChannel::HandleSendFile(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void>*>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct.event_.GetErrorCode()));
        return;
    }
    future->Complete();
}

void sharpen::WinNetStreamChannel::HandleConnect(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void>*>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct.event_.GetErrorCode()));
        return;
    }
    ::setsockopt(reinterpret_cast<SOCKET>(this->handle_),SOL_SOCKET,SO_UPDATE_CONNECT_CONTEXT,nullptr,0);
    future->Complete();
}

void sharpen::WinNetStreamChannel::HandlePoll(sharpen::WSAOverlappedStruct &olStruct)
{
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void>*>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent())
    {
        future->Fail(sharpen::MakeSystemErrorPtr(olStruct.event_.GetErrorCode()));
        return;
    }
    future->Complete();
}

#endif