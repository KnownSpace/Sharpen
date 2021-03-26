#include <sharpen/EpollSelector.hpp>

#ifdef SHARPEN_HAS_EPOLL

sharpen::EpollSelector::EpollSelector()
    :epoll_()
    ,eventfd_(0,EFD_CLOEXEC | EFD_NONBLOCK)
    ,map_()
    ,lock_()
    ,count_(8)
{
    //register event fd
    Event &event = (this->map[this->eventfd.GetHandle()] = std::move(Event()));
    event.epollEvent.data.ptr = &event;
    event.epollEvent.events = EPOLLIN;
    event.internalEventfd = true;
    this->epoll.Add(this->eventfd.GetHandle(),&(event.epollEvent));

}

bool sharpen::EpollSelector::CheckChannel(sharpen::ChannelPtr channel) noexcept
{
     return channel && channel->GetHandle() != -1 && channel->GetHandle() != nullptr;
}


void sharpen::EpollSelector::Select(EventVector &events)
{
    std::vector<sharpen::Epoll::Event> ev(std::min<sharpen::Size>(this->count_,128));
    sharpen::Uint32 count = this->epoll.Wait(ev.data(),ev.size(),-1);
    if (count == this->count_)
    {
        this->count_ <<= 1;
    }
    for (size_t i = 0; i < count; i++)
    {
        
    }
}
        
void sharpen::EpollSelector::Notify()
{
    this->eventfd_.Write(1);
}
        
void sharpen::EpollSelector::Resister(WeakChannelPtr channel)
{
    if (channel.expires())
    {
        return;
    }
    
    sharpen::ChannelPtr ch = channel.lock();
    std::unique_lock<Lock> lock(this->lock_);
    Event &event = (this->map[ch->GetHandle()] = std::move(Event()));
    lock.unlock();
    event.ioEvent_.SetChannel(ch);
    event.epollEvent.data.ptr = &event;
    event.epollEvent.events = EPOLLIN;
    event.internalEventfd = false;
    this->epoll.Add(ch->GetHandle(),&(event.epollEvent));
}
        
void sharpen::EpollSelector::EnableWriteListen(sharpen::ChannelPtr channel)
{
    if (channel.expires())
    {
        return;
    }

    sharpen::ChannelPtr ch = channel.lock();
    Event &event = this->map[ch->GetHandle()];
    event.epollEvent.events |= EPOLLOUT;
    this->epoll.Update(ch.GetHandle(),&(event.epollEvent));
}
        
void sharpen::EpollSelector::DisableWritelisten(sharpen::ChannelPtr channel)
{
    if (channel.expires())
    {
        return;
    }

    sharpen::ChannelPtr ch = channel.lock();
    Event &event = this->map[ch->GetHandle()];
    event.epollEvent.events &= ~EPOLLOUT;
    this->epoll.Update(ch.GetHandle(),&(event.epollEvent));
}

#endif