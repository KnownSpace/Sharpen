#include <sharpen/PosixNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_POSIXSOCKET

#include <sharpen/SystemError.hpp>
#include <sharpen/EventLoop.hpp>
#include <sys/mman.h>
#include <algorithm>
#include <cassert>

sharpen::PosixNetStreamChannel::PosixNetStreamChannel(sharpen::FileHandle handle)
    : Mybase()
    ,readable_(false)
    ,writeable_(false)
    ,status_(sharpen::PosixNetStreamChannel::IoStatus::Io)
    ,reader_()
    ,writer_()
    ,acceptCb_()
    ,connectCb_()
    ,pollReadCbs_()
    ,pollWriteCbs_()
{
    this->handle_ = handle;
}

bool sharpen::PosixNetStreamChannel::IsAcceptBlock(sharpen::ErrorCode err) noexcept
{
#ifdef EAGAIN
    sharpen::ErrorCode blocking = EAGAIN;
#else
    sharpen::ErrorCode blocking = EWOULDBLOCK;
#endif
    return err == blocking || err == EINTR || err == EPROTO || err == ECONNABORTED;
}

sharpen::FileHandle sharpen::PosixNetStreamChannel::DoAccept()
{
    sharpen::FileHandle s = ::accept4(this->handle_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    return s;
}

void sharpen::PosixNetStreamChannel::DoRead()
{
    bool blocking;
    bool executed;
    this->reader_.Execute(this->handle_,executed,blocking);
    this->readable_ = !executed || !blocking;
}

void sharpen::PosixNetStreamChannel::DoWrite()
{
    bool blocking;
    bool executed;
    this->writer_.Execute(this->handle_,executed,blocking);
    this->writeable_ = !executed || !blocking;
}

void sharpen::PosixNetStreamChannel::DoPollRead()
{
    if(this->pollReadCbs_.empty())
    {
        return;
    }
    iovec io;
    io.iov_base = nullptr;
    io.iov_len = 0;
    ssize_t size = ::readv(this->handle_,&io,1);
    for (auto begin = this->pollReadCbs_.begin();begin != this->pollReadCbs_.end();++begin)
    {
        (*begin)(size);
    }
    this->pollReadCbs_.clear();
}

void sharpen::PosixNetStreamChannel::DoPollWrite()
{
    if(this->pollWriteCbs_.empty())
    {
        return;
    }
    iovec io;
    io.iov_base = nullptr;
    io.iov_len = 0;
    ssize_t size = ::writev(this->handle_,&io,1);
    for (auto begin = this->pollWriteCbs_.begin();begin != this->pollWriteCbs_.end();++begin)
    {
        (*begin)(size);
    }
    this->pollWriteCbs_.clear();
}

void sharpen::PosixNetStreamChannel::HandleAccept()
{
    this->readable_ = true;
    if(this->acceptCb_)
    {
        AcceptCallback cb;
        sharpen::FileHandle accept = this->DoAccept();
        if (accept == -1)
        {
            sharpen::ErrorCode err = sharpen::GetLastError();
            if (sharpen::PosixNetStreamChannel::IsAcceptBlock(err))
            {
                this->readable_ = false;
                return;
            }
        }
        std::swap(this->acceptCb_,cb);
        cb(accept);
    }
}

void sharpen::PosixNetStreamChannel::HandleRead()
{
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Accept)
    {
        this->HandleAccept();
        return;
    }
    this->DoPollRead();
    this->DoRead();
}

bool sharpen::PosixNetStreamChannel::HandleConnect()
{
    int err{0};
    socklen_t errSize = sizeof(err);
    ::getsockopt(this->handle_,SOL_SOCKET,SO_ERROR,&err,&errSize);
    ConnectCallback cb;
    std::swap(cb,this->connectCb_);
    assert(cb);
    errno = err;
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
    cb();
    return err == 0;
}

void sharpen::PosixNetStreamChannel::HandleWrite()
{
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Connect)
    {
        bool continuable = this->HandleConnect();
        if(!continuable)
        {
            return;
        }
    }
    this->DoPollWrite();
    this->DoWrite();
}

void sharpen::PosixNetStreamChannel::TryRead(char *buf,sharpen::Size bufSize,Callback cb)
{
    this->reader_.AddPendingTask(buf,bufSize,std::move(cb));
    if (this->readable_)
    {
        this->DoRead();
    }
}

void sharpen::PosixNetStreamChannel::TryWrite(const char *buf,sharpen::Size bufSize,Callback cb)
{
    this->writer_.AddPendingTask(const_cast<char*>(buf),bufSize,std::move(cb));
    if(this->writeable_)
    {
        this->DoWrite();
    }
}

void sharpen::PosixNetStreamChannel::TryPollRead(Callback cb)
{
    this->pollReadCbs_.push_back(std::move(cb));
    if (this->readable_)
    {
        this->DoPollRead();
    }
}

void sharpen::PosixNetStreamChannel::TryPollWrite(Callback cb)
{
    this->pollWriteCbs_.push_back(std::move(cb));
    if(this->writeable_)
    {
        this->DoPollWrite();
    }
}

void sharpen::PosixNetStreamChannel::TryAccept(AcceptCallback cb)
{
    if (this->readable_)
    {
        sharpen::FileHandle handle = this->DoAccept();
        if (handle == -1)
        {
            sharpen::ErrorCode err = sharpen::GetLastError();
            if (!sharpen::PosixNetStreamChannel::IsAcceptBlock(err))
            {
                cb(handle);
                return;
            }
        }
        else
        {
            cb(handle);
            return;
        }
    }
    this->readable_ = false;
    this->acceptCb_ = std::move(cb);
}

void sharpen::PosixNetStreamChannel::TryConnect(const sharpen::IEndPoint &endPoint,ConnectCallback cb)
{
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Connect;
    int r = ::connect(this->handle_,endPoint.GetAddrPtr(),endPoint.GetAddrLen());
    if(r == -1)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != EINPROGRESS)
        {
            this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
            cb();
            return;
        }
        this->connectCb_ = std::move(cb);
        return;
    }
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
    cb();
}

void sharpen::PosixNetStreamChannel::RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    using FnPtr = void(*)(sharpen::Future<sharpen::Size> *,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),future,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryRead,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    using FnPtr = void(*)(sharpen::Future<sharpen::Size> *,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),future,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryWrite,this,buf,bufSize,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestSendFile(sharpen::FileHandle handle,sharpen::Uint64 offset,sharpen::Size size,sharpen::Future<void> *future)
{
    sharpen::Size memSize = size;
    sharpen::Size over = offset % 4096;
    if (over)
    {
        offset -= over;
    }
    memSize += over;
    over = memSize % 4096;
    if(over)
    {
        memSize += (4096-over);
    }
    void *mem =  ::mmap64(nullptr,memSize,PROT_READ,MAP_SHARED,handle,offset);
    if (mem == nullptr)
    {
        future->Fail(std::make_exception_ptr(std::bad_alloc()));
        return;
    }
    sharpen::Uintptr p = reinterpret_cast<sharpen::Uintptr>(mem);
    p += over;
    using FnPtr = void (*)(sharpen::Future<void> *,void *,sharpen::Size,sharpen::Size);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteSendFileCallback),future,mem,memSize,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryWrite,this,reinterpret_cast<const char*>(p),size,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestConnect(const sharpen::IEndPoint &endPoint,sharpen::Future<void> *future)
{
    using FnPtr = void (*)(sharpen::Future<void> *);
    ConnectCallback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteConnectCallback),future);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryConnect,this,std::cref(endPoint),std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future)
{
    using FnPtr = void (*)(sharpen::Future<sharpen::NetStreamChannelPtr> *,sharpen::FileHandle);
    AcceptCallback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteAcceptCallback),future,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryAccept,this,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestPollRead(sharpen::Future<void> *future)
{
    using FnPtr = void(*)(sharpen::Future<void> *,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompletePollCallback),future,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryPollRead,this,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::RequestPollWrite(sharpen::Future<void> *future)
{
    using FnPtr = void(*)(sharpen::Future<void> *,ssize_t);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompletePollCallback),future,std::placeholders::_1);
    this->loop_->RunInLoopSoon(std::bind(&sharpen::PosixNetStreamChannel::TryPollWrite,this,std::move(cb)));
}

void sharpen::PosixNetStreamChannel::CompleteConnectCallback(sharpen::Future<void> *future) noexcept
{
    if (sharpen::GetLastError() != 0)
    {
        //connect error
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete();
}

void sharpen::PosixNetStreamChannel::CompleteIoCallback(sharpen::Future<sharpen::Size> *future,ssize_t size) noexcept
{
    if (size == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(size);
}

void sharpen::PosixNetStreamChannel::CompletePollCallback(sharpen::Future<void> *future,ssize_t size) noexcept
{
    if (size == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete();
}

void sharpen::PosixNetStreamChannel::CompleteSendFileCallback(sharpen::Future<void> *future,void *mem,sharpen::Size memLen,ssize_t size) noexcept
{
    ::munmap(mem,memLen);
    if (size == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete();
}

void sharpen::PosixNetStreamChannel::CompleteAcceptCallback(sharpen::Future<sharpen::NetStreamChannelPtr> *future,sharpen::FileHandle accept) noexcept
{
    if (accept == -1)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(std::make_shared<sharpen::PosixNetStreamChannel>(accept));
}


void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::Char *buf, sharpen::Size bufSize, sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestWrite(buf,bufSize,&future);
}

void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset, sharpen::Future<sharpen::Size> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->WriteAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::Char *buf, sharpen::Size bufSize, sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestRead(buf,bufSize,&future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset, sharpen::Future<sharpen::Size> &future)
{
    if (buf.GetSize() < bufferOffset)
    {
        throw std::length_error("buffer size is wrong");
    }
    this->ReadAsync(buf.Data() + bufferOffset,buf.GetSize() - bufferOffset,future);
}

void sharpen::PosixNetStreamChannel::OnEvent(sharpen::IoEvent *event)
{
    if (event->IsReadEvent() || event->IsErrorEvent())
    {
        this->HandleRead();
    }
    if (event->IsWriteEvent() || event->IsErrorEvent())
    {
        this->HandleWrite();
    }
}

void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file, sharpen::Uint64 size, sharpen::Uint64 offset, sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestSendFile(file->GetHandle(),offset,size,&future);
}

void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file, sharpen::Future<void> &future)
{
    this->SendFileAsync(file,file->GetFileSize(),0,future);
}

void sharpen::PosixNetStreamChannel::AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestAccept(&future);
}

void sharpen::PosixNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint, sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestConnect(endpoint,&future);
}

void sharpen::PosixNetStreamChannel::PollReadAsync(sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestPollRead(&future);
}

void sharpen::PosixNetStreamChannel::PollWriteAsync(sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    this->RequestPollWrite(&future);
}

void sharpen::PosixNetStreamChannel::Listen(sharpen::Uint16 queueLength)
{
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Accept;
    Mybase::Listen(queueLength);
}

#endif