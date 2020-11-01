#include <sharpen/IoCompletionPort.hpp>
#ifdef SHARPEN_HAS_IOCP

#include <cassert>

#include <sharpen/SystemError.hpp>

sharpen::IoCompletionPort::IoCompletionPort()
    :handle_(::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0))
{
    if(this->handle_ == NULL)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::IoCompletionPort::~IoCompletionPort() noexcept
{
    if(this->handle_ != NULL)
    {
        ::CloseHandle(this->handle_);
    }
}

sharpen::Uint32 sharpen::IoCompletionPort::Wait(sharpen::IoCompletionPort::Event *events,sharpen::Uint32 maxEvent,sharpen::Uint32 timeout)
{
    assert(this->handle_ != NULL);
    sharpen::Uint32 count = 0;
    BOOL r = ::GetQueuedCompletionStatusEx(this->handle_,events,maxEvents,&count,timeout,FALSE);
    if(r == FALSE)
    {
        sharpen::ThrowLastError();
    }
    return count;
}

void sharpen::IoCompletionPort::Bind(sharpen::FileHandle handle)
{
    assert(this->handle_ != NULL);
    HANDLE r = ::CreateIoCompletionPort(handle,this->handle_,handle,0);
    if(r == NULL)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IoCompletionPort::Post(sharpen::IoCompletionPort::Overlapped *overlapped,sharpen::Uint32 bytesTransferred,void *completionKey)
{
    assert(this->handle_ != NULL);
    BOOL r = ::PostQueuedCompletionStatus(this->handle_,bytesTransferred,completionKey,overlapped);
    if(r == 0)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IoCompletionPort::Notify()
{
    this->Post(nullptr,0,nullptr);
}

#endif
