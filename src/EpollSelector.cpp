#include <sharpen/EpollSelector.hpp>

#ifdef SHARPEN_HAS_EPOLL

#include <mutex>
#include <cassert>

#include <fcntl.h>

sharpen::EpollSelector::EpollSelector()
    :epoll_()
    ,eventfd_(0,O_CLOEXEC | O_NONBLOCK)
    ,map_()
    ,eventBuf_(8)
#ifdef SHARPEN_HAS_IOURING
    ,ring_(nullptr)
    ,cqes_(8)
#endif
{
    //register event fd
    this->RegisterInternalEventFd(this->eventfd_.GetHandle());
#ifdef SHARPEN_HAS_IOURING
    if(sharpen::TestIoUring())
    {
        ring_.reset(new sharpen::IoUringQueue());
        this->RegisterInternalEventFd(this->ring_->EventFd().GetHandle());
    }
#endif
}

void sharpen::EpollSelector::RegisterInternalEventFd(int fd)
{
    Event &event = (this->map_[fd] = std::move(Event()));
    event.epollEvent_.data.ptr = &event;
    event.epollEvent_.events = EPOLLIN | EPOLLET;
    event.internalEventfd_ = true;
    this->epoll_.Add(fd,&(event.epollEvent_));
}

bool sharpen::EpollSelector::CheckChannel(sharpen::ChannelPtr channel) noexcept
{
     return channel && channel->GetHandle() != -1;
}


void sharpen::EpollSelector::Select(EventVector &events)
{
    sharpen::Uint32 count = this->epoll_.Wait(this->eventBuf_.data(),this->eventBuf_.size(),-1);
    for (sharpen::Size i = 0; i != count;++i)
    {
        auto &e = this->eventBuf_[i];
        auto *event = reinterpret_cast<Event*>(e.data.ptr);
        if (!event->internalEventfd_)
        {
            sharpen::Uint32 eventMask = e.events;
            sharpen::Uint32 eventType = 0;
            if (eventMask & EPOLLIN)
            {
                eventType |= sharpen::IoEvent::EventTypeEnum::Read;
            }
            if (eventMask & EPOLLOUT)
            {
                eventType |= sharpen::IoEvent::EventTypeEnum::Write;
            }
            if (eventMask & EPOLLERR)
            {
                eventType |= sharpen::IoEvent::EventTypeEnum::Error;
            }
            if (eventMask & EPOLLHUP)
            {
                eventType |= sharpen::IoEvent::EventTypeEnum::Read;
            }
            event->ioEvent_.SetEvent(eventType);
            events.push_back(&(event->ioEvent_));
        }
    }
    if(count == this->eventBuf_.size())
    {
        this->eventBuf_.resize(count * 2);
    }
#ifdef SHARPEN_HAS_IOURING
    if(this->ring_)
    {
        sharpen::Size size = this->ring_->GetCompletionStatus(this->cqes_.data(),this->cqes_.size());
        for (sharpen::Size i = 0; i != size; ++i)
        {
            sharpen::IoUringStruct *st = reinterpret_cast<sharpen::IoUringStruct*>(this->cqes_[i].user_data);
            if(this->cqes_[i].res < 0)
            {
                st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Error);
                st->event_.SetErrorCode(-this->cqes_[i].res);
            }
            else
            {
                st->event_.AddEvent(sharpen::IoEvent::EventTypeEnum::Completed);
                st->length_ = this->cqes_[i].res;
            }
            events.push_back(&(st->event_));
        }
        if(size == this->cqes_.size())
        {
            this->cqes_.resize(size * 2);
        }
    }
#endif
}
        
void sharpen::EpollSelector::Notify()
{
    this->eventfd_.Write(1);
}
        
void sharpen::EpollSelector::Resister(WeakChannelPtr channel)
{
    sharpen::ChannelPtr ch = channel.lock();
    if(!ch)
    {
        return;
    }
    Event &event = (this->map_[ch->GetHandle()] = std::move(Event()));
    event.ioEvent_.SetChannel(ch);
    event.epollEvent_.data.ptr = &event;
    event.epollEvent_.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR;
    event.internalEventfd_ = false;
    this->epoll_.Add(ch->GetHandle(),&(event.epollEvent_));
}

#ifdef SHARPEN_HAS_IOURING
sharpen::IoUringQueue *sharpen::EpollSelector::GetRing() const noexcept
{
    return this->ring_.get();
}
#endif

#endif