#include <sharpen/PosixNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_POSIXSOCKET

#include <sharpen/SystemError.hpp>
#include <sharpen/EventLoop.hpp>
#include <sys/mman.h>
#include <algorithm>

sharpen::PosixNetStreamChannel::PosixNetStreamChannel(sharpen::FileHandle handle)
    : Mybase()
    ,writeLock_()
    ,readLock_()
    ,readable_(false)
    ,writeable_(false)
    ,status_(sharpen::PosixNetStreamChannel::IoStatus::Io)
    ,error_(false)
    ,closed_(false)
    ,acceptCount_(0)
    ,readBuffers_()
    ,pendingReadBuffers_()
    ,writeBuffers_()
    ,pendingWriteBuffers_()
    ,acceptCb_()
    ,connectCb_()
    ,readCbs_()
    ,pendingReadCbs_()
    ,writeCbs_()
    ,pendingWriteCbs_()
{
    this->handle_ = handle;
}

sharpen::PosixNetStreamChannel::~PosixNetStreamChannel() noexcept
{
    this->HandleClose();
}

sharpen::Size sharpen::PosixNetStreamChannel::SetIovecs(iovec *vecs, Buffers &buf)
{
    sharpen::Size size{2};
    if (buf.size() < 2)
    {
        vecs[0].iov_base = buf.front().iov_base;
        vecs[0].iov_len = buf.front().iov_len;
        size = 1;
    }
    else
    {
        auto begin = buf.cbegin();
        vecs[0].iov_base = begin->iov_base;
        vecs[0].iov_len = begin->iov_len;
        begin++;
        vecs[1].iov_base = begin->iov_base;
        vecs[1].iov_len = begin->iov_len;
    }
    return size;
}

sharpen::Size sharpen::PosixNetStreamChannel::ConvertBytesToBufferNumber(sharpen::Size bytes, sharpen::Size &lastOffset, Buffers &buf)
{
    sharpen::Size size{0};
    auto begin = buf.cbegin();
    while (bytes > begin->iov_len)
    {
        size += 1;
        bytes -= begin->iov_len;
        begin++;
    }
    lastOffset = bytes;
    return size;
}

sharpen::Size sharpen::PosixNetStreamChannel::DoWrite(sharpen::Size &lastSize)
{
    iovec vecs[2];
    sharpen::Size size = sharpen::PosixNetStreamChannel::SetIovecs(vecs, this->writeBuffers_);
    ssize_t bytes = ::writev(this->handle_, vecs, size);
    if (bytes == -1)
    {
        this->error_ = true;
        return 0;
    }
    sharpen::Size completedNumber = sharpen::PosixNetStreamChannel::ConvertBytesToBufferNumber(bytes, lastSize, this->writeBuffers_);
    return completedNumber;
}

sharpen::Size sharpen::PosixNetStreamChannel::DoRead(sharpen::Size &lastSize)
{
    iovec vecs[2];
    sharpen::Size size = sharpen::PosixNetStreamChannel::SetIovecs(vecs, this->readBuffers_);
    ssize_t bytes = ::readv(this->handle_, vecs, size);
    if (bytes == -1)
    {
        this->error_ = true;
        return 0;
    }
    else if (bytes == 0)
    {
        return 0;
    }
    sharpen::Size completed = sharpen::PosixNetStreamChannel::ConvertBytesToBufferNumber(bytes, lastSize, this->readBuffers_);
    return completed;
}

void sharpen::PosixNetStreamChannel::FillBuffer(Buffers &buf, Buffers &pending)
{
    if (pending.empty())
    {
        return;
    }
    sharpen::Size size = buf.size();
    if (size == 0)
    {
        std::swap(pending, buf);
    }
    else if (size < 2)
    {
        std::move(pending.begin(), pending.end(), std::back_inserter(buf));
    }
}

void sharpen::PosixNetStreamChannel::FillCallback(Callbacks &cbs,Callbacks &pending)
{
    if (pending.empty())
    {
        return;
    }
    std::move(pending.begin(),pending.end(),std::back_inserter(cbs));
}

sharpen::FileHandle sharpen::PosixNetStreamChannel::DoAccept()
{
    sharpen::FileHandle s = ::accept4(this->handle_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    return s;
}

bool sharpen::PosixNetStreamChannel::ErrorBlocking()
{
#ifdef EAGAIN
    return sharpen::GetLastError() == EAGAIN;
#else
    return sharpen::GetLastError() == EWOULDBLOCK;
#endif
}

void sharpen::PosixNetStreamChannel::HandleAccept()
{
    if (!this->acceptCb_)
    {
        this->acceptCount_ += 1;
        {
            std::unique_lock<Lock> lock(this->readLock_);
            this->readable_ = true;
            return;
        }
    }
    sharpen::FileHandle s = sharpen::PosixNetStreamChannel::DoAccept();
    if (this->error_)
    {
        //blocking
        if (sharpen::PosixNetStreamChannel::ErrorBlocking())
        {
            this->acceptCount_ = 0;
            this->error_ = false;
            return;
        }
        //error
        this->acceptCb_(-1);
        this->error_ = false;
        //reset callback
        AcceptCallback emptyCb;
        this->acceptCb_ = std::move(emptyCb);
        return;
    }
    this->acceptCount_ -= 1;
    this->acceptCb_(s);
    if (this->acceptCount_ != 0)
    {
        std::unique_lock<Lock> lock(this->readLock_);
        this->readable_ = true;
    }
    //reset callback
    AcceptCallback emptyCb;
    this->acceptCb_ = std::move(emptyCb);
}

void sharpen::PosixNetStreamChannel::HandleClose() noexcept
{
    this->closed_ = true;
    Callbacks cbs;
    AcceptCallback acb;
    ConnectCallback ccb;
    {
        std::unique_lock<Lock> lock(this->readLock_);
        this->readBuffers_.clear();
        this->pendingReadBuffers_.clear();
        this->FillCallback(this->readCbs_,this->pendingReadCbs_);
        std::swap(cbs,this->readCbs_);
        acb = std::move(this->acceptCb_);
    }
    for (auto begin = cbs.begin(),end = cbs.end();begin != end;++begin)
    {
        errno = ECONNABORTED;
        if (*begin)
        {
            (*begin)(0);
        }
    }
    cbs.clear();
    if (acb)
    {
        errno = EBADFD;
        acb(-1);
    }
    {
        std::unique_lock<Lock> lock(this->writeLock_);
        this->writeBuffers_.clear();
        this->pendingWriteBuffers_.clear();
        this->FillCallback(this->writeCbs_,this->pendingWriteCbs_);
        std::swap(cbs,this->writeCbs_);
        ccb = std::move(this->connectCb_);
    }
    for (auto begin = cbs.begin(),end = cbs.end();begin != end;++begin)
    {
        errno = ECONNABORTED;
        if (*begin)
        {
            (*begin)(0);
        }
    }
    if (ccb)
    {
        errno = EBADFD;
        ccb();
    }
}

void sharpen::PosixNetStreamChannel::HandleRead()
{
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Accept)
    {
        //this socket is a server socket
        this->HandleAccept();
        return;
    }
    //this socket is a connected socket
    {
        std::unique_lock<Lock> lock(this->readLock_);
        sharpen::PosixNetStreamChannel::FillBuffer(this->readBuffers_, this->pendingReadBuffers_);
        sharpen::PosixNetStreamChannel::FillCallback(this->readCbs_,this->pendingReadCbs_);
        if (this->readBuffers_.empty())
        {
            this->readable_ = true;
            return;
        }
    }
    sharpen::Size lastSize;
    sharpen::Size size = this->DoRead(lastSize);
    if (this->error_)
    {
        //blocking
        if (sharpen::PosixNetStreamChannel::ErrorBlocking())
        {
            this->error_ = false;
            return;
        }
        //error
        this->readBuffers_.clear();
        for (auto begin = this->readCbs_.begin(), end = this->readCbs_.end(); begin != end; ++begin)
        {
            (*begin)(0);
        }
        this->error_ = false;
    }
    else
    {
        for (size_t i = 1; i < size; i++)
        {
            sharpen::Size bufSize = this->readBuffers_.front().iov_len;
            this->readBuffers_.pop_front();
            this->readCbs_.front()(bufSize);
            this->readCbs_.pop_front();
        }
        sharpen::Size lastBufSize = this->readBuffers_.front().iov_len;
        this->readBuffers_.pop_front();
        this->readCbs_.front()(lastSize);
        this->readCbs_.pop_front();
        {
            std::unique_lock<Lock> lock(this->readLock_);
            this->readable_ = lastSize == lastBufSize;
        }
    }
}

bool sharpen::PosixNetStreamChannel::HandleConnect()
{   
    bool continuable = false;
    if (this->connectCb_)
    {
        this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Io;
        this->connectCb_();
        ConnectCallback emptyCb;
        this->connectCb_ = std::move(emptyCb);
        continuable = true;
    }
    return continuable;
}

void sharpen::PosixNetStreamChannel::HandleWrite()
{
    if (this->status_ == sharpen::PosixNetStreamChannel::IoStatus::Connect)
    {
        bool continuable = this->HandleConnect();
        if (!continuable)
        {
            {
                std::unique_lock<Lock> lock(this->writeLock_);
                this->writeable_ = true;
            }
            return;
        }
    }
    {
        std::unique_lock<Lock> lock(this->writeLock_);
        this->FillBuffer(this->writeBuffers_,this->pendingWriteBuffers_);
        this->FillCallback(this->writeCbs_,this->pendingWriteCbs_);
        if (this->writeBuffers_.empty())
        {
            this->writeable_ = true;
            return;
        }
    }
    sharpen::Size lastSize;
    sharpen::Size complete = sharpen::PosixNetStreamChannel::DoWrite(lastSize);
    for (size_t i = 1; i < complete; i++)
    {
        sharpen::Size bufSize = this->writeBuffers_.front().iov_len;
        this->writeBuffers_.pop_front();
        this->writeCbs_.front()(bufSize);
        this->writeCbs_.pop_front();
    }
    sharpen::Size lastBufSize = this->writeBuffers_.front().iov_len;
    if (lastBufSize == lastSize)
    {
        this->writeBuffers_.pop_front();
        this->writeCbs_.front()(lastBufSize);
        this->writeCbs_.pop_front();
        {
            std::unique_lock<Lock> lock(this->writeLock_);
            this->writeable_ = true;
        }
        return;
    }
    sharpen::Uintptr p = reinterpret_cast<sharpen::Uintptr>(this->writeBuffers_.front().iov_base);
    p += lastSize;
    this->writeBuffers_.front().iov_len -= lastSize;
}

void sharpen::PosixNetStreamChannel::RequestRead(char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    iovec vec;
    vec.iov_base = buf;
    vec.iov_len = bufSize;
    using FnPtr = void(*)(sharpen::Future<sharpen::Size> *,sharpen::Size);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),future,std::placeholders::_1);
    bool readable = false;
    {
        std::unique_lock<Lock> lock(this->readLock_);
        std::swap(readable,this->readable_);
        this->pendingReadBuffers_.push_back(std::move(vec));
        this->pendingReadCbs_.push_back(std::move(cb));
    }
    if (readable)
    {
        this->loop_->QueueInLoop(std::bind(&sharpen::PosixNetStreamChannel::HandleRead,this));
    }
}

void sharpen::PosixNetStreamChannel::RequestWrite(const char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> *future)
{
    iovec vec;
    vec.iov_base = const_cast<char*>(buf);
    vec.iov_len = bufSize;
    using FnPtr = void(*)(sharpen::Future<sharpen::Size> *,sharpen::Size);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteIoCallback),future,std::placeholders::_1);
    bool writeable = false;
    {
        std::unique_lock<Lock> lock(this->writeLock_);
        std::swap(writeable,this->writeable_);
        this->pendingWriteBuffers_.push_back(std::move(vec));
        this->pendingWriteCbs_.push_back(std::move(cb));
    }
    if (writeable)
    {
        this->loop_->QueueInLoop(std::bind(&sharpen::PosixNetStreamChannel::HandleWrite,this));
    }
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
    iovec vec;
    vec.iov_len = size;
    vec.iov_base = reinterpret_cast<void*>(p);
    using FnPtr = void (*)(sharpen::Future<void> *,void *,sharpen::Size,sharpen::Size);
    Callback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteSendFileCallback),future,mem,memSize,std::placeholders::_1);
    bool writeable = false;
    {
        std::unique_lock<Lock> lock(this->writeLock_);
        std::swap(writeable,this->writeable_);
        this->pendingWriteBuffers_.push_back(std::move(vec));
        this->pendingWriteCbs_.push_back(std::move(cb));
    }
    if (writeable)
    {
        this->loop_->QueueInLoop(std::bind(&sharpen::PosixNetStreamChannel::HandleWrite,this));
    }
}

void sharpen::PosixNetStreamChannel::RequestConnect(const sharpen::IEndPoint &endPoint,sharpen::Future<void> *future)
{
    int r = ::connect(this->handle_,endPoint.GetAddrPtr(),endPoint.GetAddrLen());
    if (r == -1)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (err != EINPROGRESS)
        {
            future->Fail(sharpen::MakeSystemErrorPtr(err));
            return;
        }
        bool writeable = false;
        using FnPtr = void (*)(sharpen::Future<void> *);
        ConnectCallback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteConnectCallback),future);
        {
            std::unique_lock<Lock> lock(this->writeLock_);
            this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Connect;
            std::swap(writeable,this->writeable_);
            this->connectCb_ = std::move(cb);
        }
        if (writeable)
        {
            this->loop_->QueueInLoop(std::bind(&sharpen::PosixNetStreamChannel::HandleWrite,this));
        }
    }
    future->Complete();
}

void sharpen::PosixNetStreamChannel::RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future)
{
    bool readable = false;
    using FnPtr = void (*)(sharpen::Future<sharpen::NetStreamChannelPtr> *,sharpen::FileHandle);
    AcceptCallback cb = std::bind(reinterpret_cast<FnPtr>(&sharpen::PosixNetStreamChannel::CompleteAcceptCallback),future,std::placeholders::_1);
    {
        std::unique_lock<Lock> lock(this->readLock_);
        std::swap(readable,this->readable_);
        this->acceptCb_ = std::move(cb);    
    }
    if (readable)
    {
        this->loop_->QueueInLoop(std::bind(&sharpen::PosixNetStreamChannel::HandleRead,this));
    }
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

void sharpen::PosixNetStreamChannel::CompleteIoCallback(sharpen::Future<sharpen::Size> *future,sharpen::Size size) noexcept
{
    if (sharpen::GetLastError() != 0)
    {
        future->Fail(sharpen::MakeLastErrorPtr());
        return;
    }
    future->Complete(size);
}

void sharpen::PosixNetStreamChannel::CompleteSendFileCallback(sharpen::Future<void> *future,void *mem,sharpen::Size memLen,sharpen::Size) noexcept
{
    sharpen::ErrorCode err = sharpen::GetLastError();
    ::munmap(mem,memLen);
    if (err != 0)
    {
        future->Fail(sharpen::MakeSystemErrorPtr(err));
        return;
    }
    future->Complete();
}

void sharpen::PosixNetStreamChannel::CompleteAcceptCallback(sharpen::Future<sharpen::NetStreamChannelPtr> *future,sharpen::FileHandle accept) noexcept
{
    if (sharpen::GetLastError() != 0 && accept == -1)
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
    if (this->closed_)
    {
        future.Fail(sharpen::MakeSystemErrorPtr(EBADFD));
    }
    this->RequestWrite(buf,bufSize,&future);
}

void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf, sharpen::Future<sharpen::Size> &future)
{
    this->WriteAsync(buf.Data(),buf.GetSize(),future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::Char *buf, sharpen::Size bufSize, sharpen::Future<sharpen::Size> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    if (this->closed_)
    {
        future.Fail(sharpen::MakeSystemErrorPtr(EBADFD));
    }
    this->RequestRead(buf,bufSize,&future);
}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf, sharpen::Future<sharpen::Size> &future)
{
    this->ReadAsync(buf.Data(),buf.GetSize(),future);
}

void sharpen::PosixNetStreamChannel::OnEvent(sharpen::IoEvent *event)
{
    if(this->closed_)
    {
        return;
    }
    if (event->IsReadEvent())
    {
        this->HandleRead();
    }
    if (event->IsWriteEvent())
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
    if (this->closed_)
    {
        future.Fail(sharpen::MakeSystemErrorPtr(EBADFD));
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
    if (this->closed_)
    {
        future.Fail(sharpen::MakeSystemErrorPtr(EBADFD));
    }
    this->RequestAccept(&future);
}

void sharpen::PosixNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint, sharpen::Future<void> &future)
{
    if (!this->IsRegistered())
    {
        throw std::logic_error("should register to a loop first");
    }
    if (this->closed_)
    {
        future.Fail(sharpen::MakeSystemErrorPtr(EBADFD));
    }
    this->RequestConnect(endpoint,&future);
}

void sharpen::PosixNetStreamChannel::Listen(sharpen::Uint16 queueLength)
{
    this->status_ = sharpen::PosixNetStreamChannel::IoStatus::Accept;
    Mybase::Listen(queueLength);
}

#endif