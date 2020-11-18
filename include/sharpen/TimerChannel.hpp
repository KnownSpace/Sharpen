#pragma once
#ifndef _SHARPEN_TIMERCHANNEL_HPP
#define _SHARPEN_TIMERCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "SystemMacro.hpp"

namespace sharpen
{

    class TimerChannel:public sharpen::IChannel,public sharpen::Noncopyable
    {
    private:
    
        sharpen::FileHandle handle_;
        
        sharpen::EventLoop *loop_;
    public:
    };
}

#endif
