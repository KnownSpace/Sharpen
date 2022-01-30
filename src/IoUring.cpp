#include <sharpen/IoUring.hpp>

#ifdef SHARPEN_HAS_IOURING

#include <cstring>
#include <atomic>

#include <sys/mman.h>

sharpen::IoUring::IoUring(std::uint32_t entries,std::uint32_t flags,std::uint32_t sq_thread_cpu,std::uint32_t sq_thread_idle,std::uint32_t cq_entries)
    :ringFd_(-1)
    ,busyNumber_(0)
    ,sringAddr_(nullptr)
    ,sring_()
    ,sringSize_(0)
    ,sqes_(nullptr)
    ,sqesSize_(0)
    ,cringAddr_(nullptr)
    ,cring_()
    ,cringSize_(0)
{
    struct io_uring_params p;
    std::memset(&p,0,sizeof(p));
    p.flags = flags;
    p.sq_thread_cpu = sq_thread_cpu;
    p.sq_thread_idle = sq_thread_idle;\
    p.cq_entries = cq_entries;
    this->ringFd_ = sharpen::IoUringSetup(entries,&p);
    if(this->ringFd_ == -1)
    {
        sharpen::ThrowLastError();
    }
    //init sqe and cqe
    this->sringSize_ = p.sq_off.array + p.sq_entries*sizeof(unsigned);
    this->cringSize_ = p.cq_off.cqes + p.cq_entries*sizeof(struct io_uring_cqe);
    //check single mmap
    if(p.features & IORING_FEAT_SINGLE_MMAP)
    {
        if(this->cringSize_ > this->sringSize_)
        {
            this->sringSize_ = this->cringSize_;
        }
        this->cringSize_ = this->sringSize_;
    }
    //mmap sring
    void *sring = ::mmap(nullptr,sringSize_,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,this->ringFd_,IORING_OFF_SQ_RING);
    if(sring == MAP_FAILED)
    {
        ::close(this->ringFd_);
        sharpen::ThrowLastError();
    }
    //mmap cring
    void *cring{nullptr};
    if(p.features & IORING_FEAT_SINGLE_MMAP)
    {
        cring = sring;
    }
    else
    {
        cring = ::mmap(nullptr,this->cringSize_,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,this->ringFd_,IORING_OFF_CQ_RING);
        if(cring == MAP_FAILED)
        {
            ::munmap(sring,this->sringSize_);
            ::close(this->ringFd_);
            sharpen::ThrowLastError();
        }
    }
    //init sring and cring
    {
        char *addr = reinterpret_cast<char*>(sring);
        this->sring_.array_ = reinterpret_cast<unsigned*>(addr + p.sq_off.array);
        this->sring_.flags_ = reinterpret_cast<unsigned*>(addr + p.sq_off.flags);
        this->sring_.head_ = reinterpret_cast<unsigned*>(addr + p.sq_off.head);
        this->sring_.tail_ = reinterpret_cast<unsigned*>(addr + p.sq_off.tail);
        this->sring_.ringMask_ = reinterpret_cast<unsigned*>(addr + p.sq_off.ring_mask);
        this->sring_.ringEntries_ = reinterpret_cast<unsigned*>(addr + p.sq_off.ring_entries);
        this->sringAddr_ = addr;
    }
    {
        char *addr = reinterpret_cast<char*>(cring);
        this->cring_.head_ = reinterpret_cast<unsigned*>(addr + p.cq_off.head);
        this->cring_.tail_ = reinterpret_cast<unsigned*>(addr + p.cq_off.tail);
        this->cring_.ringMask_ = reinterpret_cast<unsigned*>(addr + p.cq_off.ring_mask);
        this->cring_.ringEntires_ = reinterpret_cast<unsigned*>(addr + p.cq_off.ring_entries);
        this->cring_.cqes_ = reinterpret_cast<struct io_uring_cqe*>(addr + p.cq_off.cqes);
        this->cringAddr_ = addr;
    }
    //mmap sqes
    this->sqesSize_ = p.sq_entries * sizeof(struct io_uring_sqe);
    sring = ::mmap(nullptr,this->sqesSize_,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,this->ringFd_, IORING_OFF_SQES);
    if(sring == MAP_FAILED)
    {
        ::munmap(this->sringAddr_,this->sringSize_);
        ::munmap(this->cringAddr_,this->cringSize_);
        ::close(this->ringFd_);
        sharpen::ThrowLastError();
    }
    this->sqes_ = reinterpret_cast<struct io_uring_sqe*>(sring);
}

sharpen::IoUring::~IoUring() noexcept
{
    if(this->ringFd_ != -1)
    {
        ::close(this->ringFd_);
    }
    if(this->sringAddr_)
    {
        ::munmap(this->sringAddr_,this->sringSize_);
    }
    if(this->sringAddr_ != this->cringAddr_ && this->cringAddr_)
    {
        ::munmap(this->cringAddr_,this->cringSize_);
    }
    if(this->sqes_)
    {
        ::munmap(this->sqes_,this->sqesSize_);
    }
}

void sharpen::IoUring::Enter(unsigned int to_submit,unsigned int min_complete,unsigned int flags,sigset_t *sig)
{
    if(sharpen::IoUringEnter(this->ringFd_,to_submit,min_complete,flags,sig) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IoUring::Enter(unsigned int to_submit,unsigned int min_complete,unsigned int flags,void *arg,size_t argsz)
{
    if(sharpen::IoUringEnterEx(this->ringFd_,to_submit,min_complete,flags,arg,argsz) == -1)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IoUring::SubmitToSring(const struct io_uring_sqe *sqe)
{
    unsigned index,tail;
    tail = *this->sring_.tail_;
    index = tail & *this->sring_.ringMask_;
    struct io_uring_sqe *s = &this->sqes_[index];
    std::memcpy(s,sqe,sizeof(*s));
    this->sring_.array_[index] = index;
    tail += 1;
    //hope it work
    reinterpret_cast<std::atomic_uint*>(this->sring_.tail_)->store(tail,std::memory_order::memory_order_release);
}

bool sharpen::IoUring::GetFromCring(struct io_uring_cqe *cqe)
{
    unsigned head{0};
    //hope it work
    head = reinterpret_cast<std::atomic_uint*>(this->cring_.head_)->load(std::memory_order::memory_order_acquire);
    if(head == *this->cring_.tail_)
    {
        return false;
    }
    std::memcpy(cqe,this->cring_.cqes_ + (head & (*this->cring_.ringMask_)),sizeof(*cqe));
    head += 1;
    reinterpret_cast<std::atomic_uint*>(this->cring_.head_)->store(head,std::memory_order::memory_order_release);
    return true;
}

#endif