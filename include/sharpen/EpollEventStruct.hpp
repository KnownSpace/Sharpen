#pragma once
#ifndef _SHARPEN_EPOLLEVENTSTRUCT_HPP
#define _SHARPEN_EPOLLEVENTSTRUCT_HPP

#include "Epoll.hpp"

#ifdef SHARPEN_HAS_EPOLL

#include "IChannel.hpp"
#include "IoEvent.hpp"

namespace sharpen
{
    struct EpollEventStruct
    {
        using WeakChannelPtr = std::weak_ptr<sharpen::IChannel>;
        using EpollEvent = ::epoll_event;

        WeakChannelPtr channel_;

        EpollEvent epollEvent_;

        sharpen::IoEvent ioEvent_;

        bool internalEventfd_;
    };
}

#endif

#endif