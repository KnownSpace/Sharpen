#pragma once
#ifndef _SHARPEN_IOCPOVERLAPPEDSTRUCT_HPP
#define _SHARPEN_IOCPOVERLAPPEDSTRUCT_HPP

#include "IoCompletionPort.hpp"

#ifdef SHARPEN_HAS_IOCP

#include "IChannel.hpp"
#include "IoEvent.hpp"

namespace sharpen
{
    struct IocpOverlappedStruct
    {
        OVERLAPPED ol_;
        sharpen::IoEvent event_;
        void *data_;
        std::size_t length_;
        sharpen::ChannelPtr channel_;
    };
}   // namespace sharpen

#endif
#endif