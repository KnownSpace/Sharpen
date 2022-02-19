#pragma once
#ifndef _SHARPEN_IOURING_HPP
#define _SHARPEN_IOURING_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_LINUX
#include <sys/syscall.h>

#ifdef __NR_io_uring_setup
#define SHARPEN_HAS_IOURING
#endif

#ifdef SHARPEN_HAS_IOURING

#include <cstdint>
#include <atomic>

#include <unistd.h>
#include <linux/io_uring.h>

#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "EventFd.hpp"
#include "SystemError.hpp"

namespace sharpen
{
    //system call
    inline int IoUringSetup(std::uint32_t entries, struct io_uring_params *p)
    {
        return (int)syscall(SYS_io_uring_setup,entries,p);
    }

    inline int IoUringEnter(int ring_fd,unsigned int to_submit,unsigned int min_complete,unsigned int flags,sigset_t *sig)
    {
        return (int)syscall(SYS_io_uring_enter,ring_fd,to_submit,min_complete,flags,sig);
    }

    inline int IoUringEnterEx(unsigned int fd, unsigned int to_submit,unsigned int min_complete, unsigned int flags,const void *arg, size_t argsz)
    {
        return (int)syscall(SYS_io_uring_enter,fd,to_submit,min_complete,flags,arg,argsz);
    }

    inline int IoUringRegister(unsigned int fd,unsigned int opcode,void *arg,unsigned int nr_args)
    {
        return (int)syscall(SYS_io_uring_register,fd,opcode,arg,nr_args);
    }

    struct IoSring 
    {
        unsigned *head_;
        unsigned *tail_;
        unsigned *ringMask_;
        unsigned *ringEntries_;
        unsigned *flags_;
        unsigned *array_;
    };

    struct IoCring 
    {
        unsigned *head_;
        unsigned *tail_;
        unsigned *ringMask_;
        unsigned *ringEntires_;
        struct io_uring_cqe *cqes_;
    };

    class IoUring:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::IoUring;
    

        template<typename _T>
        inline static _T Load(const _T *p) noexcept
        {
            return std::atomic_load_explicit(reinterpret_cast<const std::atomic<_T>*>(p),std::memory_order_acquire);
        }

        template<typename _T>
        inline static void Store(_T *p,_T value) noexcept
        {
            std::atomic_store_explicit(reinterpret_cast<std::atomic<_T>*>(p), value,std::memory_order_release);
        }

        int ringFd_;
        void *sringAddr_;
        sharpen::IoSring sring_;
        sharpen::Size sringSize_;
        struct io_uring_sqe *sqes_;
        sharpen::Size sqesSize_;
        sharpen::Size sringNumber_;
        void *cringAddr_;
        sharpen::IoCring cring_;
        sharpen::Size cringSize_;
        sharpen::Size cringNumber_;
        sharpen::Size requestNumber_;
    public:

        IoUring(std::uint32_t entries,std::uint32_t flags,std::uint32_t sq_thread_cpu,std::uint32_t sq_thread_idle,std::uint32_t cq_entries);
    
        ~IoUring() noexcept;

        void SubmitToSring(const struct io_uring_sqe *sqe);

        bool GetFromCring(struct io_uring_cqe *cqe);

        void Enter(unsigned int to_submit,unsigned int min_complete,unsigned int flags,sigset_t *sig);

        void Enter(unsigned int to_submit,unsigned int min_complete,unsigned int flags,void *arg,size_t argsz);

        inline void Enter(unsigned int to_submit,unsigned int min_complete,unsigned int flags)
        {
            this->Enter(to_submit,min_complete,flags,nullptr);
        }

        inline void Wait(unsigned int min_complete)
        {
            this->Enter(0,min_complete,IORING_ENTER_GETEVENTS);
        }

        inline void Wait(unsigned int min_complete,sigset_t *sig)
        {
            this->Enter(0,min_complete,IORING_ENTER_GETEVENTS,sig);
        }

        bool Requestable() const;

        void RegisterEventFd(const sharpen::EventFd &eventFd);

        void UnregisterEventFd();
    };
}

#endif
#endif
#endif