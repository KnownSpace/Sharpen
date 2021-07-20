#include <sharpen/EpollSelector.hpp>

#ifdef SHARPEN_HAS_EPOLL

#include <mutex>

#include <fcntl.h>

sharpen::EpollSelector::EpollSelector()
    :epoll_()
    ,eventfd_(0,O_CLOEXEC | O_NONBLOCK)
    ,map_()
    ,eventBuf_(8)
{
    //register event fd
    Event &event = (this->map_[this->eventfd_.GetHandle()] = std::move(Event()));
    event.epollEvent_.data.ptr = &event;
    event.epollEvent_.events = EPOLLIN | EPOLLET;
    event.internalEventfd_ = true;
    this->epoll_.Add(this->eventfd_.GetHandle(),&(event.epollEvent_));

}

bool sharpen::EpollSelector::CheckChannel(sharpen::ChannelPtr channel) noexcept
{
     return channel && channel->GetHandle() != -1;
}


void sharpen::EpollSelector::Select(EventVector &events)
{
    sharpen::Uint32 count = this->epoll_.Wait(this->eventBuf_.data(),this->eventBuf_.size(),-1);
    for (size_t i = 0; i < count; i++)
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

#endif