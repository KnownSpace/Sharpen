#pragma once
#ifndef _SHARPEN_PIPECHANNEL_HPP
#define _SHARPEN_PIPECHANNEL_HPP

#include "Pipe.hpp"
#include "IChannel.hpp"

namespace sharpen
{
    class PipeChannel:public sharpen::IChannel,public sharpen::Noncopyable
    {
    private:
        
        sharpen::Pipe pipe_;
        
        sharpen::EventLoop *loop_;
    public:
    };
}

#endif
