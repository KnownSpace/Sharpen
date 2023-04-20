#include <sharpen/IocpSelector.hpp>

#ifdef SHARPEN_HAS_IOCP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <limits>
#include <mutex>
#include <cstring>

sharpen::IocpSelector::IocpSelector()
    :iocp_()
    ,eventBuf_(Self::minEventBufLength_)
{}

bool sharpen::IocpSelector::CheckChannel(sharpen::ChannelPtr &channel) noexcept
{
    return channel && channel->GetHandle() != INVALID_HANDLE_VALUE && channel->GetHandle() != nullptr;
}

void sharpen::IocpSelector::Resister(WeakChannelPtr channel)
{
    if (channel.expired())
    {
        return;
    }
    sharpen::ChannelPtr ch = channel.lock();
    if (!sharpen::IocpSelector::CheckChannel(ch))
    {
        return;
    }
    this->iocp_.Bind(ch->GetHandle());
}

void sharpen::IocpSelector::Notify()
{
    this->iocp_.Notify();
}

void sharpen::IocpSelector::Select(EventVector &events)
{
    assert(events.size() <= (std::numeric_limits<std::uint32_t>::max)());
    std::uint32_t count = this->iocp_.Wait(this->eventBuf_.data(),static_cast<std::uint32_t>(this->eventBuf_.size()),INFINITE);
    for (std::size_t i = 0; i != count; ++i)
    {
        sharpen::IoCompletionPort::Event &e = this->eventBuf_[i];
        if (e.lpOverlapped != nullptr && e.lpCompletionKey != static_cast<ULONG_PTR>(NULL))
        {
            //get overlapped struct
            sharpen::IocpOverlappedStruct *olStructPtr = CONTAINING_RECORD(e.lpOverlapped,sharpen::IocpOverlappedStruct,ol_);
            //check channel
            sharpen::ChannelPtr channel = olStructPtr->event_.GetChannel();
            if (channel)
            {
                sharpen::ErrorCode code = ERROR_SUCCESS;
                //check error
                BOOL r = ::GetOverlappedResultEx(channel->GetHandle(),e.lpOverlapped,&(e.dwNumberOfBytesTransferred),0,FALSE);
                sharpen::IoEvent::EventType type = olStructPtr->event_.GetEventType();
                type ^= sharpen::IoEvent::EventTypeEnum::Request;
                if (r == FALSE)
                {
                    code = sharpen::GetLastError();
                    type |= sharpen::IoEvent::EventTypeEnum::Error;
                    
                }
                else
                {
                    type |= sharpen::IoEvent::EventTypeEnum::Completed;
                }
                //set event
                olStructPtr->event_.SetEvent(type);
                olStructPtr->event_.SetErrorCode(code);
                //set length
                olStructPtr->length_ = e.dwNumberOfBytesTransferred;
                //set result
                events.push_back(&(olStructPtr->event_));
            }
        }
    }
    if (count == this->eventBuf_.size() && count != Self::maxEventBufLength_)
    {
        this->eventBuf_.resize(count * 2);
    }
    else
    {
        std::memset(this->eventBuf_.data(),0,count * sizeof(sharpen::IoCompletionPort::Event));
    }
}

#endif