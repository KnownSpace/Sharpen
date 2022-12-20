#include <sharpen/IPosixIoOperator.hpp>

#ifdef SHARPEN_IS_NIX

#include <cassert>
#include <mutex>

sharpen::IPosixIoOperator::IPosixIoOperator()
    :bufs_()
    ,pendingBufs_()
    ,cbs_()
    ,pendingCbs_()
    ,mark_(0)
{}

void sharpen::IPosixIoOperator::ConvertByteToBufferNumber(std::size_t byteNumber,std::size_t &bufferNumber,std::size_t &lastSize)
{
    IoBuffer *bufs = this->GetFirstBuffer();
    assert(bufs != nullptr);
    std::size_t remaining = this->GetRemainingSize();
    assert(remaining != 0);
    (void)remaining;
    std::size_t i{0};
    while (bufs[i].iov_len < byteNumber)
    {
        assert(i < remaining);
        byteNumber -= bufs[i].iov_len;
        i += 1;
    }
    bufferNumber = i;
    lastSize = byteNumber;
}

void sharpen::IPosixIoOperator::FillBufferAndCallback()
{
    if (this->GetRemainingSize() == 0)
    {
        this->bufs_.clear();
        this->cbs_.clear();
        this->MoveMark(0);
        std::swap(this->bufs_,this->pendingBufs_);
        std::swap(this->cbs_,this->pendingCbs_);
    }
}

void sharpen::IPosixIoOperator::MoveMark(std::size_t newMark)
{
    assert(this->bufs_.size() == this->cbs_.size());
    //assert(this->bufs_.size() >= newMark);
    this->mark_ = newMark;
}

std::size_t sharpen::IPosixIoOperator::GetMark() const
{
    return this->mark_;
}

const sharpen::IPosixIoOperator::IoBuffer *sharpen::IPosixIoOperator::GetFirstBuffer() const
{
    if (this->GetRemainingSize())
    {
        const IoBuffer *buf = this->bufs_.data();
        return buf + this->mark_;
    }
    return nullptr;
}

sharpen::IPosixIoOperator::IoBuffer *sharpen::IPosixIoOperator::GetFirstBuffer()
{
    if (this->GetRemainingSize())
    {
        IoBuffer *buf = this->bufs_.data();
        return buf + this->mark_;
    }
    return nullptr;
}

const sharpen::IPosixIoOperator::Callback *sharpen::IPosixIoOperator::GetFirstCallback() const
{
    if (this->GetRemainingSize())
    {
        const Callback *cb = this->cbs_.data();
        return cb + this->mark_;
    }
    return nullptr;
}

sharpen::IPosixIoOperator::Callback *sharpen::IPosixIoOperator::GetFirstCallback()
{
    if (this->GetRemainingSize())
    {
        Callback *cb = this->cbs_.data();
        return cb + this->mark_;
    }
    return nullptr;
}

void sharpen::IPosixIoOperator::AddPendingTask(char *buf,std::size_t size,sharpen::IPosixIoOperator::Callback cb)
{
    IoBuffer buffer;
    buffer.iov_base = buf;
    buffer.iov_len = size;
    this->pendingBufs_.push_back(std::move(buffer));
    assert(cb);
    this->pendingCbs_.push_back(std::move(cb));
}

std::size_t sharpen::IPosixIoOperator::GetRemainingSize() const
{
    assert(this->bufs_.size() == this->cbs_.size());
    assert(this->bufs_.size() >= this->mark_);
    return  this->bufs_.size() - this->mark_;
}

void sharpen::IPosixIoOperator::Execute(sharpen::FileHandle handle,bool &executed,bool &blocking)
{
    this->FillBufferAndCallback();
    this->NviExecute(handle,executed,blocking);
}

bool sharpen::IPosixIoOperator::IsBlockingError(sharpen::ErrorCode err)
{
#ifdef EAGAIN
    return err == EAGAIN;    
#else
    return err == EWOULDBLOCK;
#endif
}

std::size_t sharpen::IPosixIoOperator::ComputePendingSize() const
{
    std::size_t size{0};
    const IoBuffer *buf = this->GetFirstBuffer();
    for (size_t i = 0,count = this->GetRemainingSize(); i != count; ++i)
    {
        size += buf->iov_len;
    }
    return size;
}

void sharpen::IPosixIoOperator::CancelAllIo(sharpen::ErrorCode err) noexcept
{
    this->FillBufferAndCallback();
    Callback *cbs = this->GetFirstCallback();
    std::size_t size = this->GetRemainingSize();
    errno = err;
    for (size_t i = 0; i != size; ++i)
    {
        cbs[i](-1);
    }
    size += this->GetMark();
    this->MoveMark(size);
    this->FillBufferAndCallback();
    cbs = this->GetFirstCallback();
    size = this->GetRemainingSize();
    for (std::size_t i = 0; i != size; ++i)
    {
        cbs[i](-1);
    }
    size += this->GetMark();
    this->MoveMark(size);
}

#endif