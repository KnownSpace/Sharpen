#pragma once
#ifndef _SHARPEN_WRITEREADYEVENT_HPP
#define _SHARPEN_WRITEREADYEVENT_HPP

#include "IoEvent.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    class WriteReadyEvent:public sharpen::IoEvent
    {
    private:
        using Base = sharpen::IoEvent;
        
        sharpen::Size bufSize_;
        const sharpen::Char *buf_;
    public:
        WriteReadyEvent(sharpen::IChannel *channel,sharpen::Size bufSize,sharpen::Char *buf);
        
        ~WriteReadyEvent() = default;
        
        virtual void Handle(sharpen::ISelector *selector) override;
    };
}

#endif
