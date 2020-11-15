#pragma once
#ifndef _SHARPEN_EVENTFDCHANNEL_HPP
#define _SHARPEN_EVENTFDCHANNEL_HPP

#include "EventFd.hpp"

#ifdef SHARPEN_HAS_EVENTFD

#include "IChannel.hpp"

namespace sharpen
{
    class EventFdChannel:public sharpen::IChannel,public sharpen::Noncopyable
    {
    private:

        sharpen::EventFd eventFd_;
 
        sharpen::EventLoop *loop_;
    public:

    };
}

#endif
#endif
