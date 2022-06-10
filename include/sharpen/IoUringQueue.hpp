#pragma once
#ifndef _SHARPEN_IOURINGQUEUE_HPP
#define _SHARPEN_IOURINGQUEUE_HPP

#include "IoUring.hpp"

#ifdef SHARPEN_HAS_IOURING

#include <vector>
#include <deque>

namespace sharpen
{
    class IoUringQueue
    {
    private:
        using Self = sharpen::IoUringQueue;
        using Cqe = struct io_uring_cqe;
        using Sqe = struct io_uring_sqe;
        using CompletionQueue = std::vector<Cqe>;
        using SubmitQueue = std::deque<Sqe>;

        static constexpr std::size_t queueLength{64};
    
        sharpen::EventFd eventFd_;
        sharpen::IoUring ring_;
        CompletionQueue compQueue_;
        SubmitQueue subQueue_;

        void Submit();
    public:
    
        IoUringQueue();

        explicit IoUringQueue(bool blockEventFd);
    
        ~IoUringQueue() noexcept;

        sharpen::EventFd &EventFd() noexcept
        {
            return this->eventFd_;
        }

        const sharpen::EventFd &EventFd() const noexcept
        {
            return this->eventFd_;
        }

        void SubmitIoRequest(const Sqe &sqe);

        std::size_t GetCompletionStatus(Cqe *cqes,std::size_t size);
    };

    extern bool TestIoUring() noexcept;
}

#endif
#endif