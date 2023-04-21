#include <sharpen/IoUringQueue.hpp>

#ifdef SHARPEN_HAS_IOURING

#include <cstring>

#include <fcntl.h>

#include <sharpen/IteratorOps.hpp>

sharpen::IoUringQueue::IoUringQueue()
    : IoUringQueue(false)
{
}

sharpen::IoUringQueue::IoUringQueue(bool blockEventFd)
    : eventFd_(0, O_CLOEXEC | (blockEventFd ? 0 : O_NONBLOCK))
    , ring_(Self::queueLength_, 0, 0, 0, 0)
    , compQueue_()
    , subQueue_()
{
    this->compQueue_.reserve(reservedCqSize_);
    this->ring_.RegisterEventFd(this->eventFd_);
}

sharpen::IoUringQueue::~IoUringQueue() noexcept
{
    this->ring_.UnregisterEventFd();
}

void sharpen::IoUringQueue::Submit()
{
    if (this->subQueue_.empty())
    {
        return;
    }
    auto ite = this->subQueue_.begin();
    std::size_t moved{0};
    while (this->ring_.Requestable())
    {
        this->ring_.SubmitToSring(&*ite);
        ++ite;
        ++moved;
    }
    this->subQueue_.erase(this->subQueue_.begin(), ite);
    this->ring_.Enter(moved, 0, 0);
}

void sharpen::IoUringQueue::SubmitIoRequest(const Sqe &sqe)
{
    if (this->ring_.Requestable())
    {
        this->ring_.SubmitToSring(&sqe);
        this->ring_.Enter(1, 0, 0);
        return;
    }
    this->subQueue_.emplace_back(sqe);
    return;
}

std::size_t sharpen::IoUringQueue::GetCompletionStatus(Cqe *cqes, std::size_t size)
{
    std::size_t cqeNum{0};
    if (!this->compQueue_.empty())
    {
        cqeNum = (std::min)(this->compQueue_.size(), size);
        auto begin = this->compQueue_.rbegin();
        auto end = sharpen::IteratorForward(begin, cqeNum);
        std::size_t index{0};
        while (begin != end)
        {
            std::memcpy(cqes + index, std::addressof(*begin), sizeof(*begin));
            ++begin;
            ++index;
        }
        this->compQueue_.erase(sharpen::IteratorBackward(this->compQueue_.end(), cqeNum),
                               this->compQueue_.end());
    }
    bool moreEv{false};
    while (cqeNum != size)
    {
        moreEv = this->ring_.GetFromCring(cqes + cqeNum);
        if (!moreEv)
        {
            break;
        }
        ++cqeNum;
    }
    while (moreEv)
    {
        this->compQueue_.emplace_back();
        moreEv = this->ring_.GetFromCring(&this->compQueue_.back());
        if (!moreEv)
        {
            this->compQueue_.pop_back();
        }
    }
    this->Submit();
    return cqeNum;
}

bool sharpen::TestIoUring() noexcept
{
    static int status{0};
    if (!status)
    {
        try
        {
            sharpen::IoUringQueue queue;
            status = 1;
            static_cast<void>(queue);
        }
        catch (const std::system_error &ignore)
        {
            (void)ignore;
            status = 2;
        }
    }
    return status == 1;
}
#endif