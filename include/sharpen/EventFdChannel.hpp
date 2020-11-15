#pragma once
#ifndef _SHARPEN_EVENTFDCHANNEL_HPP
#define _SHARPEN_EVENTFDCHANNEL_HPP

#include "EventFd.hpp"

#ifdef SHARPEN_HAS_EVENTFD

#include "IChannel.hpp"

namespace sharpen
{
    class EventFdChannel:public sharpen::IChannel
    {
    private:
    public:
    };
}

#endif
#endif
