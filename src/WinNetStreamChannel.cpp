#include <sharpen/WinNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_WINSOCKET

#include <MSWSock.h>
#include <WinSock2.h>
#include <sharpen/EventLoop.hpp>
#include <cassert>
#include <cstring>
#include <new>
#include <type_traits>

void sharpen::WinNetStreamChannel::InitOverlapped(OVERLAPPED &ol) {
    std::memset(&ol, 0, sizeof(ol));
}

void sharpen::WinNetStreamChannel::InitOverlappedStruct(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::WinNetStreamChannel::InitOverlapped(olStruct.ol_);
    // init buf
    olStruct.buf_.buf = nullptr;
    olStruct.buf_.len = 0;
    // init accepted socket
    olStruct.accepted_ = reinterpret_cast<sharpen::FileHandle>(INVALID_SOCKET);
    // init length
    olStruct.length_ = 0;
    // set olStruct data
    olStruct.event_.SetEvent(sharpen::IoEvent::EventTypeEnum::Request);
    olStruct.event_.SetChannel(this->shared_from_this());
    olStruct.event_.SetErrorCode(ERROR_SUCCESS);
    olStruct.channel_ = this->shared_from_this();
}

void sharpen::WinNetStreamChannel::Closer(sharpen::FileHandle handle) noexcept {
    SOCKET sock = reinterpret_cast<SOCKET>(handle);
    ::closesocket(sock);
}

sharpen::WinNetStreamChannel::WinNetStreamChannel(sharpen::FileHandle handle, int af)
    : Mybase()
    , af_(af) {
    assert(handle != reinterpret_cast<sharpen::FileHandle>(INVALID_SOCKET));
    this->handle_ = handle;
    using FnPtr = void (*)(sharpen::FileHandle);
    this->closer_ =
        std::bind(static_cast<FnPtr>(&sharpen::WinNetStreamChannel::Closer), std::placeholders::_1);
}

void sharpen::WinNetStreamChannel::RequestWrite(const char *buf,
                                                std::size_t bufSize,
                                                sharpen::Future<std::size_t> *future) {
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Write);
    // record future
    olStruct->data_ = future;
    // set buf
    olStruct->buf_.buf = const_cast<CHAR *>(buf);
    olStruct->buf_.len = static_cast<ULONG>(bufSize);
    // request
    BOOL r = ::WSASend(reinterpret_cast<SOCKET>(this->handle_),
                       &(olStruct->buf_),
                       1,
                       nullptr,
                       0,
                       reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),
                       nullptr);
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            if (err == sharpen::ErrorCancel || err == sharpen::ErrorConnectionAborted ||
                err == sharpen::ErrorConnectionReset || err == sharpen::ErrorNotSocket) {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf,
                                              std::size_t bufferOffset,
                                              sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, future);
}

void sharpen::WinNetStreamChannel::RequestRead(char *buf,
                                               std::size_t bufSize,
                                               sharpen::Future<std::size_t> *future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Read);
    // record future
    olStruct->data_ = future;
    // set buf
    olStruct->buf_.buf = buf;
    olStruct->buf_.len = static_cast<ULONG>(bufSize);
    // request
    static DWORD recvFlag = 0;
    BOOL r = ::WSARecv(reinterpret_cast<SOCKET>(this->handle_),
                       &(olStruct->buf_),
                       1,
                       nullptr,
                       &recvFlag,
                       reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),
                       nullptr);
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            if (err == sharpen::ErrorCancel || err == sharpen::ErrorConnectionAborted ||
                err == sharpen::ErrorConnectionReset || err == sharpen::ErrorNotSocket) {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf,
                                             std::size_t bufferOffset,
                                             sharpen::Future<std::size_t> &future) {
    if (buf.GetSize() < bufferOffset) {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset, buf.GetSize() - bufferOffset, future);
}

void sharpen::WinNetStreamChannel::OnEvent(sharpen::IoEvent *event) {
    std::unique_ptr<sharpen::WSAOverlappedStruct> olStruct(
        reinterpret_cast<sharpen::WSAOverlappedStruct *>(event->GetData()));
    sharpen::IoEvent::EventType ev = olStruct->event_.GetEventType();
    if (ev & sharpen::IoEvent::EventTypeEnum::Accept) {
        this->HandleAccept(*olStruct);
    } else if (ev &
               (sharpen::IoEvent::EventTypeEnum::Read | sharpen::IoEvent::EventTypeEnum::Write)) {
        this->HandleReadAndWrite(*olStruct);
    } else if (ev & sharpen::IoEvent::EventTypeEnum::Connect) {
        this->HandleConnect(*olStruct);
    } else if (ev & sharpen::IoEvent::EventTypeEnum::Sendfile) {
        this->HandleSendFile(*olStruct);
    } else if (ev & sharpen::IoEvent::EventTypeEnum::Poll) {
        this->HandlePoll(*olStruct);
    }
}

void sharpen::WinNetStreamChannel::RequestSendFile(sharpen::FileChannelPtr file,
                                                   std::uint64_t size,
                                                   std::uint64_t offset,
                                                   sharpen::Future<std::size_t> *future) {
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc{}));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Sendfile);
    // record future
    olStruct->data_ = future;
    // set offset
    LARGE_INTEGER li;
    li.QuadPart = offset;
    olStruct->ol_.Offset = li.LowPart;
    olStruct->ol_.OffsetHigh = li.HighPart;
    // request
    BOOL r = ::TransmitFile(reinterpret_cast<SOCKET>(this->handle_),
                            file->GetHandle(),
                            static_cast<DWORD>(size),
                            0,
                            &(olStruct->ol_),
                            nullptr,
                            0);
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            if (err == sharpen::ErrorCancel || err == sharpen::ErrorConnectionAborted ||
                err == sharpen::ErrorConnectionReset || err == sharpen::ErrorNotSocket) {
                future->Complete(static_cast<std::size_t>(0));
                return;
            }
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,
                                                 sharpen::Future<std::size_t> &future) {
    this->SendFileAsync(file, file->GetFileSize(), 0, future);
}

void sharpen::WinNetStreamChannel::RequestAccept(
    sharpen::Future<sharpen::NetStreamChannelPtr> *future) {
    static std::atomic<LPFN_ACCEPTEX> AcceptEx{nullptr};
    // get acceptex
    if (AcceptEx == nullptr) {
        LPFN_ACCEPTEX wsaAcceptEx{nullptr};
        GUID acceptexId = WSAID_ACCEPTEX;
        DWORD dwBytes;
        int iResult = WSAIoctl(reinterpret_cast<SOCKET>(this->handle_),
                               SIO_GET_EXTENSION_FUNCTION_POINTER,
                               &acceptexId,
                               sizeof(acceptexId),
                               &wsaAcceptEx,
                               sizeof(wsaAcceptEx),
                               &dwBytes,
                               NULL,
                               NULL);
        if (iResult != 0) {
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
        AcceptEx.store(wsaAcceptEx);
    }
    LPFN_ACCEPTEX WSAAcceptEx{AcceptEx.load()};
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Accept);
    // record future
    olStruct->data_ = future;
    // open client socket
    SOCKET s = ::socket(this->af_, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        delete olStruct;
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    olStruct->accepted_ = reinterpret_cast<sharpen::FileHandle>(s);
    // alloc buf
    olStruct->buf_.buf = reinterpret_cast<char *>(std::calloc(1024, sizeof(char)));
    if (olStruct->buf_.buf == nullptr) {
        delete olStruct;
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // request
    BOOL r = WSAAcceptEx(reinterpret_cast<SOCKET>(this->handle_),
                         reinterpret_cast<SOCKET>(olStruct->accepted_),
                         olStruct->buf_.buf,
                         0,
                         sizeof(SOCKADDR_IN) + 16,
                         sizeof(SOCKADDR_IN) + 16,
                         nullptr,
                         &(olStruct->ol_));
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::RequestConnect(const sharpen::IEndPoint *endpoint,
                                                  sharpen::Future<void> *future) {
    static std::atomic<LPFN_CONNECTEX> ConnectEx{nullptr};
    // get connectex
    if (ConnectEx == nullptr) {
        LPFN_CONNECTEX wsaConnectEx{nullptr};
        GUID connectexId = WSAID_CONNECTEX;
        DWORD dwBytes;
        int iResult = WSAIoctl(reinterpret_cast<SOCKET>(this->handle_),
                               SIO_GET_EXTENSION_FUNCTION_POINTER,
                               &connectexId,
                               sizeof(connectexId),
                               &wsaConnectEx,
                               sizeof(wsaConnectEx),
                               &dwBytes,
                               NULL,
                               NULL);
        if (iResult != 0) {
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
        ConnectEx.store(wsaConnectEx);
    }
    LPFN_CONNECTEX WSAConnectEx{ConnectEx.load()};
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Connect);
    // record future
    olStruct->data_ = future;
    // request
    BOOL r = WSAConnectEx(reinterpret_cast<SOCKET>(this->handle_),
                          endpoint->GetAddrPtr(),
                          endpoint->GetAddrLen(),
                          nullptr,
                          0,
                          nullptr,
                          &(olStruct->ol_));
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::RequestPollRead(sharpen::Future<void> *future) {
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Poll);
    // record future
    olStruct->data_ = future;
    // set buf
    olStruct->buf_.buf = nullptr;
    olStruct->buf_.len = 0;
    // request
    static DWORD recvFlag = 0;
    // use WSARecv
    // with buf = nullptr & len = 0
    BOOL r = ::WSARecv(reinterpret_cast<SOCKET>(this->handle_),
                       &(olStruct->buf_),
                       1,
                       nullptr,
                       &recvFlag,
                       reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),
                       nullptr);
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::RequestPollWrite(sharpen::Future<void> *future) {
    sharpen::WSAOverlappedStruct *olStruct = new (std::nothrow) sharpen::WSAOverlappedStruct();
    if (!olStruct) {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    // init iocp olStruct
    this->InitOverlappedStruct(*olStruct);
    olStruct->event_.SetData(olStruct);
    olStruct->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Poll);
    // record future
    olStruct->data_ = future;
    // set buf
    olStruct->buf_.buf = nullptr;
    olStruct->buf_.len = 0;
    // request
    // use WSASend
    // with buf = nullptr & len = 0
    BOOL r = ::WSASend(reinterpret_cast<SOCKET>(this->handle_),
                       &(olStruct->buf_),
                       1,
                       nullptr,
                       0,
                       reinterpret_cast<LPWSAOVERLAPPED>(&(olStruct->ol_)),
                       nullptr);
    if (r != TRUE) {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != ERROR_IO_PENDING && err != ERROR_SUCCESS) {
            delete olStruct;
            future->Fail(sharpen::MakeLastErrorPtr());
            return;
        }
    }
}

void sharpen::WinNetStreamChannel::HandleReadAndWrite(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::Future<std::size_t> *future =
        reinterpret_cast<sharpen::Future<std::size_t> *>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent()) {
        sharpen::ErrorCode code{olStruct.event_.GetErrorCode()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorConnectionAborted ||
            code == sharpen::ErrorConnectionReset || code == sharpen::ErrorNotSocket) {
            future->Complete(static_cast<std::size_t>(0));
            return;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    future->Complete(olStruct.length_);
}

void sharpen::WinNetStreamChannel::HandleAccept(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::Future<sharpen::NetStreamChannelPtr> *future =
        reinterpret_cast<sharpen::Future<sharpen::NetStreamChannelPtr> *>(olStruct.data_);
    std::free(olStruct.buf_.buf);
    if (olStruct.event_.IsErrorEvent()) {
        sharpen::ErrorCode code{olStruct.event_.GetErrorCode()};
        if (code == sharpen::ErrorNotSocket) {
            code = sharpen::ErrorConnectionAborted;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    ::setsockopt(reinterpret_cast<SOCKET>(olStruct.accepted_),
                 SOL_SOCKET,
                 SO_UPDATE_ACCEPT_CONTEXT,
                 reinterpret_cast<char *>(&this->handle_),
                 sizeof(SOCKET));
    sharpen::NetStreamChannelPtr channel =
        std::make_shared<sharpen::WinNetStreamChannel>(olStruct.accepted_, this->af_);
    future->Complete(channel);
}

void sharpen::WinNetStreamChannel::HandleSendFile(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::Future<std::size_t> *future =
        reinterpret_cast<sharpen::Future<std::size_t> *>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent()) {
        sharpen::ErrorCode code{olStruct.event_.GetErrorCode()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorConnectionAborted ||
            code == sharpen::ErrorConnectionRefused || code == sharpen::ErrorNotSocket) {
            future->Complete(static_cast<std::size_t>(0));
            return;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    future->Complete(olStruct.length_);
}

void sharpen::WinNetStreamChannel::HandleConnect(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void> *>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent()) {
        sharpen::ErrorCode code{olStruct.event_.GetErrorCode()};
        if (code == sharpen::ErrorNotSocket) {
            code = sharpen::ErrorConnectionAborted;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    ::setsockopt(
        reinterpret_cast<SOCKET>(this->handle_), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
    future->Complete();
}

void sharpen::WinNetStreamChannel::HandlePoll(sharpen::WSAOverlappedStruct &olStruct) {
    sharpen::Future<void> *future = reinterpret_cast<sharpen::Future<void> *>(olStruct.data_);
    if (olStruct.event_.IsErrorEvent()) {
        sharpen::ErrorCode code{olStruct.event_.GetErrorCode()};
        if (code == sharpen::ErrorCancel || code == sharpen::ErrorConnectionAborted ||
            code == sharpen::ErrorConnectionRefused || code == sharpen::ErrorNotSocket) {
            future->Complete();
            return;
        }
        future->Fail(sharpen::MakeSystemErrorPtr(code));
        return;
    }
    future->Complete();
}

void sharpen::WinNetStreamChannel::ReadAsync(char *buf,
                                             std::size_t bufSize,
                                             sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->loop_->RunInLoop(
        std::bind(&sharpen::WinNetStreamChannel::RequestRead, this, buf, bufSize, &future));
}

void sharpen::WinNetStreamChannel::WriteAsync(const char *buf,
                                              std::size_t bufSize,
                                              sharpen::Future<std::size_t> &future) {
    assert(buf != nullptr || (buf == nullptr && bufSize == 0));
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->loop_->RunInLoop(
        std::bind(&sharpen::WinNetStreamChannel::RequestWrite, this, buf, bufSize, &future));
}

void sharpen::WinNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,
                                                 std::uint64_t size,
                                                 std::uint64_t offset,
                                                 sharpen::Future<std::size_t> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Complete(static_cast<std::size_t>(0));
        return;
    }
    this->loop_->RunInLoop(std::bind(
        &sharpen::WinNetStreamChannel::RequestSendFile, this, file, size, offset, &future));
}

void sharpen::WinNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint,
                                                sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Fail(sharpen::MakeSystemErrorPtr(sharpen::ErrorConnectionAborted));
        return;
    }
    this->loop_->RunInLoop(
        std::bind(&sharpen::WinNetStreamChannel::RequestConnect, this, &endpoint, &future));
}

void sharpen::WinNetStreamChannel::AcceptAsync(
    sharpen::Future<sharpen::NetStreamChannelPtr> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Fail(sharpen::MakeSystemErrorPtr(sharpen::ErrorCancel));
        return;
    }
    this->loop_->RunInLoop(std::bind(&sharpen::WinNetStreamChannel::RequestAccept, this, &future));
}

void sharpen::WinNetStreamChannel::PollReadAsync(sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Complete();
        return;
    }
    this->loop_->RunInLoop(
        std::bind(&sharpen::WinNetStreamChannel::RequestPollRead, this, &future));
}

void sharpen::WinNetStreamChannel::PollWriteAsync(sharpen::Future<void> &future) {
    if (!this->IsRegistered()) {
        throw std::logic_error("should register to a loop first");
    }
    if (this->handle_ == INVALID_HANDLE_VALUE) {
        future.Complete();
        return;
    }
    this->loop_->RunInLoop(
        std::bind(&sharpen::WinNetStreamChannel::RequestPollWrite, this, &future));
}

void sharpen::WinNetStreamChannel::RequestCancel() noexcept {
    ::CancelIo(this->handle_);
}

void sharpen::WinNetStreamChannel::Cancel() noexcept {
    this->loop_->RunInLoopSoon(std::bind(&sharpen::WinNetStreamChannel::RequestCancel, this));
}

#endif