#pragma once
#ifndef _SHARPEN_WINSOCKETCHANNEL_HPP
#define _SHARPEN_WINSOCKETCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#define SHARPEN_USE_WINSOCKET

#include "ISocketChannel.hpp"

namespace sharpen
{
    class WinSocketChannel:public sharpen::ISocketChannel,public sharpen::Noncopyable
    {
    private:
        sharpen::EventLoop *loop_;
        
        sharpen::FileHandle handle_;
    public:
    };
    
    using NativeSocketChannel = sharpen::WinSocketChannel;
};

#endif
#endif
