#pragma once
#ifndef _SHARPEN_FILECHANNELIMPL_HPP
#define _SHARPEN_FILECHANNELIMPL_HPP

#include "WinFileChannel.hpp"
#include "PosixFileChannel.hpp"

namespace sharpen
{
#ifdef SHARPEN_HAS_WINFILE
    using FileChannelImpl = sharpen::WinFileChannel;
#else
    using FileChannelImpl = sharpen::PosixFileChannel;
#endif
}

#endif