#pragma once
#ifndef _SHARPEN_IOURINGQUEUE_HPP
#define _SHARPEN_IOURINGQUEUE_HPP

#include "IoUring.hpp"

#ifdef SHARPEN_HAS_IOURING

#include <vector>

namespace sharpen
{
    class IoUringQueue
    {
    private:
        using Self = sharpen::IoUringQueue;
        using Cqe = struct io_uring_cqe;
        using Sqe = struct io_uring_sqe;
        using CompletionQueue = std::vector<Cqe>;
        using SubmitQueue = std::vector<Sqe>;

        static constexpr sharpen::Size queueLength{256};
    
        sharpen::EventFd eventFd_;
        sharpen::IoUring ring_;
        CompletionQueue compQueue_;
        SubmitQueue subQueue_;

        void Submit();
    public:
    
        IoUringQueue();
    
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

        sharpen::Size GetCompletionStatus(Cqe *cqes,sharpen::Size size);
    };

    extern bool TestIoUring() noexcept;
}

#endif
#endif