#pragma once
#ifndef _SHARPEN_IOURINGSTRUCT_HPP
#define _SHARPEN_IOURINGSTRUCT_HPP

#include "IoUringQueue.hpp" // IWYU pragma: keep

#ifdef SHARPEN_HAS_IOURING

#include "IoEvent.hpp"
#include <sys/uio.h>

namespace sharpen
{
    struct IoUringStruct
    {
        sharpen::IoEvent event_;
        void *data_;
        std::size_t length_;
        iovec vec_;
        sharpen::ChannelPtr channel_;
    };
}   // namespace sharpen
#endif
#endif