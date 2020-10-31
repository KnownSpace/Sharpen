#prgama once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    class IChannel
    {
    private:
        using Self = sharpen::IChannel;
        
    public:
        using EventCode = sharpen::Uint32;
        
        IChannel() = default;
        
        IChannel(const Self &) = default;
        
        IChannel(Self &&) noexcept = default;
        
        virtual ~IChannel() = default;
        
        //it will be called when a io operation was completed
        virtual void OnComplete(EventCode code) = 0;
        
        //close channel
        virtual void Close() = 0;
    };
}

#endif
