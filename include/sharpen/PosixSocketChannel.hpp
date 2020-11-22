#pragma once
#ifndef _SHARPEN_POSIXSOCKETCHANNEL_HPP
#define _SHARPEN_POSIXSOCKETCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_USE_POSIXSOCKET

#include "ISocketChannel.hpp"

namespace sharpen
{
    class PosixSocketChannel:public sharpen::ISocketChannel,public sharpen::Noncopyable
    {
    private:
        sharpen::EventLoop *loop_;
        
        sharpen::FileHandle handle_;
    public:
    
    };
    
    using NativeSocketChannel = sharpen::PosixSocketChannel;
};

#endif
#endif
