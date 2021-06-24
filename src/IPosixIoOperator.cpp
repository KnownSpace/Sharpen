#include <sharpen/IPosixIoOperator.hpp>

#ifdef SHARPEN_IS_NIX

#include <cassert>
#include <mutex>

sharpen::IPosixIoOperator::IPosixIoOperator()
    :lock_()
    ,bufs_()
    ,pendingBufs_()
    ,cbs_()
    ,pendingCbs_()
    ,mark_(0)
{}

void sharpen::IPosixIoOperator::ConvertByteToBufferNumber(sharpen::Size byteNumber,sharpen::Size &bufferNumber,sharpen::Size &lastSize)
{
    IoBuffer *bufs = this->GetFirstBuffer();
    assert(bufs != nullptr);
    sharpen::Size remaining = this->GetRemainingSize();
    assert(remaining != 0);
    sharpen::Size number{0};
    sharpen::Size i{0};
    while (bufs[i].iov_len < byteNumber)
    {
        assert(i < remaining);
        byteNumber -= bufs[i].iov_len;
        ++i;
    }
    bufferNumber = i;
    lastSize = byteNumber;
}

void sharpen::IPosixIoOperator::FillBufferAndCallback()
{
    {
        std::unique_lock<Lock> lock(this->lock_);
        if (!this->GetRemainingSize())
        {
            this->bufs_.clear();
            this->cbs_.clear();
            std::swap(this->bufs_,this->pendingBufs_);
            std::swap(this->cbs_,this->pendingCbs_);
            this->mark_ = 0;
        }
    }
}

void sharpen::IPosixIoOperator::FillBufferAndCallback(OnEmpty onEmpty)
{
    {
        std::unique_lock<Lock> lock(this->lock_);
        if (!this->GetRemainingSize())
        {
            this->bufs_.clear();
            this->cbs_.clear();
            std::swap(this->bufs_,this->pendingBufs_);
            std::swap(this->cbs_,this->pendingCbs_);
            this->mark_ = 0;
            if (this->bufs_.empty())
            {
                assert(this->cbs_.empty());
                onEmpty();
            }
            
        }
    }
}

void sharpen::IPosixIoOperator::MoveMark(sharpen::Size newMark)
{
    this->mark_ = newMark;
}

sharpen::Size sharpen::IPosixIoOperator::GetMark() const
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

void sharpen::IPosixIoOperator::AddPendingTask(sharpen::Char *buf,sharpen::Size size,sharpen::IPosixIoOperator::Callback cb)
{
    {
        std::unique_lock<Lock> lock(this->lock_);
        IoBuffer buffer;
        buffer.iov_base = buf;
        buffer.iov_len = size;
        this->pendingBufs_.push_back(std::move(buffer));
        assert(cb);
        this->pendingCbs_.push_back(std::move(cb));
    }
}

void sharpen::IPosixIoOperator::AddPendingTask(sharpen::Char *buf,sharpen::Size size,sharpen::IPosixIoOperator::Callback cb,ExecuteInLock doInLock)
{
    {
        std::unique_lock<Lock> lock(this->lock_);
        IoBuffer buffer;
        buffer.iov_base = buf;
        buffer.iov_len = size;
        this->pendingBufs_.push_back(std::move(buffer));
        assert(cb);
        this->pendingCbs_.push_back(std::move(cb));
        doInLock();
    }
}

sharpen::Size sharpen::IPosixIoOperator::GetRemainingSize() const
{
    assert(this->bufs_.size() == this->cbs_.size());
    return this->bufs_.size() - this->mark_;
}

void sharpen::IPosixIoOperator::Execute(sharpen::FileHandle handle,OnEmpty notExec,bool &blocking)
{
    this->FillBufferAndCallback(notExec);
    this->DoExecute(handle,blocking);
}

bool sharpen::IPosixIoOperator::IsBlockingError(sharpen::ErrorCode err)
{
#ifdef EAGAIN
    return err == EAGAIN;    
#else
    return err == EWOULDBLOCK;
#endif
}
#endif