#include <sharpen/EpollSelector.hpp>

#ifdef SHARPEN_HAS_EPOLL

#include <mutex>

#include <fcntl.h>

sharpen::EpollSelector::EpollSelector()
    :epoll_()
    ,eventfd_(0,O_CLOEXEC | O_NONBLOCK)
    ,map_()
    ,lock_()
    ,count_(8)
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
    std::vector<sharpen::Epoll::Event> ev(this->count_);
    sharpen::Uint32 count = this->epoll_.Wait(ev.data(),ev.size(),-1);
    if (count == this->count_ && this->count_ < 128)
    {
        this->count_ *= 2;
    }
    for (size_t i = 0; i < count; i++)
    {
        auto &e = ev[i];
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
            event->ioEvent_.SetEvent(eventType);
            events.push_back(&(event->ioEvent_));
        }
    }
}
        
void sharpen::EpollSelector::Notify()
{
    this->eventfd_.Write(1);
}
        
void sharpen::EpollSelector::Resister(WeakChannelPtr channel)
{
    if (channel.expired())
    {
        return;
    }
    
    sharpen::ChannelPtr ch = channel.lock();
    std::unique_lock<Lock> lock(this->lock_);
    Event &event = (this->map_[ch->GetHandle()] = std::move(Event()));
    lock.unlock();
    event.ioEvent_.SetChannel(ch);
    event.epollEvent_.data.ptr = &event;
    event.epollEvent_.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR;
    event.internalEventfd_ = false;
    this->epoll_.Add(ch->GetHandle(),&(event.epollEvent_));
}

#endif