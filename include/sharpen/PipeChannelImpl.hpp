#pragma once
#ifndef _SHARPEN_PIPECHANNELIMPL_HPP
#define _SHARPEN_PIPECHANNELIMPL_HPP

#include "WinInputPipeChannel.hpp"
#include "WinOutputPipeChannel.hpp"
#include "PosixInputPipeChannel.hpp"
#include "PosixOutputPipeChannel.hpp"

namespace sharpen
{
    //input pipe
#ifdef SHARPEN_HAS_WININPUTPIPE
    using InputPipeChannelImpl = sharpen::WinInputPipeChannel;
#else
    using InputPipeChannelImpl = sharpen::PosixInputPipeChannel;
#endif

    //output pipe
#ifdef SHARPEN_HAS_WINOUTPUTPIPE
    using OutputPipeChannelImpl = sharpen::WinOutputPipeChannel;
#else
    using OutputPipeChannelImpl = sharpen::PosixOutputPipeChannel;
#endif
}

#endif