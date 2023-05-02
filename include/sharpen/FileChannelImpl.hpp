#pragma once
#ifndef _SHARPEN_FILECHANNELIMPL_HPP
#define _SHARPEN_FILECHANNELIMPL_HPP

#include "PosixFileChannel.hpp"
#include "WinFileChannel.hpp"

namespace sharpen {
#ifdef SHARPEN_HAS_WINFILE
    using FileChannelImpl = sharpen::WinFileChannel;
#else
    using FileChannelImpl = sharpen::PosixFileChannel;
#endif
}   // namespace sharpen

#endif