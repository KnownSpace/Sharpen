#pragma once
#ifndef _SHARPEN_READREADYEVENT_HPP
#define _SHARPEN_READREADYEVENT_HPP

#include "IoReadyEvent.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    class ReadReadyEvent:public sharpen::IoEvent
    {
    private:
        using Base = sharpen::IoEvent;
    
        sharpen::Size bufSize_;
        sharpen::Char *buf_;
    public:
        ReadReadyEvent(sharpen::IChannel *channel,sharpen::Size bufSize,sharpen::Char *buf);
        
        ~ReadReadyEvent() = default;
        
        virtual void Handle(sharpen::ISelector &selector) override;
    };
}

#endif
