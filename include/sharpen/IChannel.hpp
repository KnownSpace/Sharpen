#prgama once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "TypeDef.hpp"
#include "FileTypeDef.hpp"

namespace sharpen
{
    class IEventLoop;

    class IoEvent;
    
    class IChannel
    {
    private:
        using Self = sharpen::IChannel;
        
    protected:
        virtual void DoClose() noexcept = 0;
        
    public:
        
        IChannel() = default;
        
        IChannel(const Self &) = default;
        
        IChannel(Self &&) noexcept = default;
        
        virtual ~IChannel() noexcept = default;
        
        //it will be called when a io operation was completed
        virtual void OnComplete(sharpen::IoEvent *event) = 0;
        
        virtual void RegisterAsync(sharpen::IEventLoop &loop) = 0;
        
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

        virtual sharpen::IEventLoop *GetLoop() = 0;
    };
}

#endif
