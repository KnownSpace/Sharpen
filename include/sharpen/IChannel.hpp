#prgama once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "TypeDef.hpp"
#include "FileTypeDef.hpp"

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
        
        virtual ~IChannel() noexcept = default;
        
        //it will be called when a io operation was completed
        virtual void OnComplete(EventCode code) = 0;
        
        virtual void RegisterAsync(ISelector &selector) = 0;
        
        virtual void UnregisterAsync() noexcept = 0;
        
        //close channel
        void Close() noexcept
        {
            //await unregister
            this->UnregisterAsync();
            //close channel
            this->DoClose();
        }
        
        virtual sharpen::FileHandle GetHandle() = 0;
    };
}

#endif
