#pragma once
#ifndef _SHARPEN_PIPECHANNEL_HPP
#define _SHARPEN_PIPECHANNEL_HPP

#include "IOutputPipeChannel.hpp"
#include "IInputPipeChannel.hpp"

namespace sharpen
{
    void OpenPipeChannel(sharpen::InputPipeChannelPtr &in,sharpen::OutputPipeChannelPtr &out);
}

#endif