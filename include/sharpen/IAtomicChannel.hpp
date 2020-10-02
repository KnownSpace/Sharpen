#pragma once
#ifndef _SHARPEN_IATOMICCHANNEL_HPP
#define _SHARPEN_IATOMICCHANNEL_HPP

#include "IChannel.hpp"

namespace sharpen
{
    class IAtomicChannel:public sharpen::IChannel
    {
    private:
        /* data */
    public:
        IAtomicChannel(/* args */);
        
        virtual ~IAtomicChannel() = 0;
    };
    
}

#endif