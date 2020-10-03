#pragma once
#ifndef _SHARPEN_IATOMICCHANNEL_HPP
#define _SHARPEN_IATOMICCHANNEL_HPP

#include "IChannel.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IAtomicChannel:public sharpen::IChannel
    {
    public:
        IAtomicChannel() = default;

        virtual ~IAtomicChannel() = 0;
    };
    
}

#endif