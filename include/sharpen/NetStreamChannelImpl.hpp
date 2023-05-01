#pragma once
#ifndef _SHARPEN_NETSTREAMCHANNELIMPL_HPP
#define _SHARPEN_NETSTREAMCHANNELIMPL_HPP

#include "PosixFileChannel.hpp"
#include "WinNetStreamChannel.hpp"

namespace sharpen {
#ifdef SHARPEN_HAS_WINSOCKET
    using NetStreamChannelImpl = sharpen::WinNetStreamChannel;
#else
    using NetStreamChannelImpl = sharpen::PosixNetStreamChannel;
#endif
}   // namespace sharpen

#endif