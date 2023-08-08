#include <sharpen/WinInputPipeChannel.hpp>

#ifdef SHARPEN_HAS_WININPUTPIPE

#include <sharpen/EventLoop.hpp>
#include <Windows.h>
#include <cassert>
#include <cstring>
#include <new>
#include <stdexcept>

sharpen::WinInputPipeChannel::WinInputPipeChannel(sharpen::FileHandle handle)
    : Mybase() {
    assert(handle != INVALID_HANDLE_VALUE);
    this->handle_ = handle;
}

sharpen::WinInputPipeChannel::~WinInputPipeChannel() noexcept {
    if (this->handle_ != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(this->handle_, nullptr);
    }
}

void sharpen::WinInputPipeChannel::InitOverlapped(OVERLAPPED &ol) {
    std::memset(&ol, 0, sizeof(ol));
}

void sharpen::WinInputPipeChannel::InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct) {
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.length_ = 0;
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinInputPipeChannel::RequestRead(char *buf,
                                               std::size_t bufSize,
                                               sharpen::Future<std::size_t> *future) {
    IocpOverlappedStruct *olStruct = new (std::nothrow) sharpen::IocpOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    olStruct->event_.SetData(olStruct);
    // record future
    olStruct->data_ = future;
    BOOL r = ::ReadFile(this->handle_, buf, static_cast<DWORD>(bufSize), nullptr, &(olStruct->ol_));
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            if (err == sharpen::ErrorBrokenPipe || err == sharpen::ErrorCancel) {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinInputPipeChannel::ReadAsync(char *buf,
                                             std::size_t bufSize,
                                             sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if(bufSize > MaxIoSize) {
        bufSize = MaxIoSize;
    }
    this->loop_->RunInLoop(std::bind(&Self::RequestRead, this, buf, bufSize, &future));
}

void sharpen::WinInputPipeChannel::ReadAsync(sharpen::ByteBuffer &buf,
                                             std::size_t bufOffset,
                                             sharpen::Future<std::size_t> &future) {
    if (bufOffset > buf.GetSize()) {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufOffset, buf.GetSize() - bufOffset, future);
}

void sharpen::WinInputPipeChannel::OnEvent(sharpen::IoEvent *event) {
    std::unique_ptr<sharpen::IocpOverlappedStruct> ev(
        reinterpret_cast<sharpen::IocpOverlappedStruct *>(event->GetData()));
    sharpen::Future<std::size_t> *future =
        reinterpret_cast<sharpen::Future<std::size_t> *>(ev->data_);
    if (event->IsErrorEvent()) {
        sharpen::ErrorCode code{event->GetErrorCode()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorBrokenPipe) {
            future->Complete(static_cast<std::size_t>(0));
            return;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    future->Complete(ev->length_);
}
#endif