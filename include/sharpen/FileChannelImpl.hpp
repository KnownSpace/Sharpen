#pragma once
#ifndef _SHARPEN_FILECHANNELIMPL_HPP
#define _SHARPEN_FILECHANNELIMPL_HPP

#include "PosixFileChannel.hpp" // IWYU pragma: keep
#include "WinFileChannel.hpp" // IWYU pragma: keep

namespace sharpen {
#ifdef SHARPEN_HAS_WINFILE
    using FileChannelImpl = sharpen::WinFileChannel;
#else
    using FileChannelImpl = sharpen::PosixFileChannel;
#endif
}   // namespace sharpen

#endif