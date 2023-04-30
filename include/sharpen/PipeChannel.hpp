#pragma once
#ifndef _SHARPEN_PIPECHANNEL_HPP
#define _SHARPEN_PIPECHANNEL_HPP

#include "IInputPipeChannel.hpp"
#include "IOutputPipeChannel.hpp"

namespace sharpen
{
    void OpenPipeChannel(sharpen::InputPipeChannelPtr &in, sharpen::OutputPipeChannelPtr &out);
}

#endif