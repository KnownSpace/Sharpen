#prgama once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    class ISelector;
    
    class IChannel
    {
    private:
        using Self = sharpen::IChannel;
        
    protected:
        virtual void DoClose() noexcept = 0;
        
    public:
        using EventCode = sharpen::Uint32;
        
        IChannel() = default;
        
        IChannel(const Self &) = default;
        
        IChannel(Self &&) noexcept = default;
        
        virtual ~IChannel() = default;
        
        //it will be called when a io operation was completed
        virtual void OnComplete(EventCode code) = 0;
        
        virtual void Register(ISelector &selector) = 0;
        
        virtual void Unregister() noexcept = 0;
        
        //close channel
        void Close() noexcept
        {
            this->Unregister();
            this->DoClose();
        }
    };
}

#endif
