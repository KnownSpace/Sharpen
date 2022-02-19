#pragma once
#ifndef _SHARPEN_IOURINGSTRUCT_HPP
#define _SHARPEN_IOURINGSTRUCT_HPP

#include "IoUringQueue.hpp"

#ifdef SHARPEN_HAS_IOURING

#include <sys/uio.h>

#include "IoEvent.hpp"

namespace sharpen
{
    struct IoUringStruct
    {
        sharpen::IoEvent event_;
        void *data_;
        sharpen::Size length_;
        iovec vec_;
        sharpen::ChannelPtr channel_;
    };
}
#endif
#endif