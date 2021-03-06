#include <sharpen/IocpSelector.hpp>
#include <algorithm>
#include<sharpen/TypeDef.hpp>

#ifdef SHARPEN_HAS_IOCP

#include <mutex>

sharpen::IocpSelector::IocpSelector()
    :iocp_()
    ,count_(8)
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
    std::vector<sharpen::IoCompletionPort::Event> ev(this->count_);
    sharpen::Uint32 count = this->iocp_.Wait(ev.data(),static_cast<Uint32>(ev.size()),INFINITE);
    if (count == this->count_ && this->count_ <128)
    {
        this->count_ *= 2;
    }
    for (size_t i = 0; i < count; i++)
    {
        sharpen::IoCompletionPort::Event &e = ev[i];
        if (e.lpOverlapped != nullptr && e.lpCompletionKey != NULL)
        {
            //get overlapped struct
            sharpen::IocpOverlappedStruct *olStructPtr = CONTAINING_RECORD(e.lpOverlapped,sharpen::IocpOverlappedStruct,ol_);
            //check channel
            sharpen::ChannelPtr channel = olStructPtr->event_.GetChannel();
            if (olStructPtr != nullptr && channel)
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
}

#endif