#pragma once
#ifndef _SHARPEN_NETSTREAMCHANNELIMPL_HPP
#define _SHARPEN_NETSTREAMCHANNELIMPL_HPP

#include "WinNetStreamChannel.hpp"
#include "PosixFileChannel.hpp"

namespace sharpen
{
#ifdef SHARPEN_HAS_WINSOCKET
    using NetStreamChannelImpl = sharpen::WinNetStreamChannel;
#else
    using NetStreamChannelImpl = sharpen::PosixNetStreamChannel;
#endif   
}

#endif