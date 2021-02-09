#pragma once
#ifndef _SHARPEN_SOCKETCHANNEL_HPP
#define _SHARPEN_SOCKETCHANNEL_HPP

#include <memory>

#include "WinSocketChannel.hpp"
#include "PosixSocketChannel.hpp"

namespace sharpen
{
    class SocketChannel:public sharpen::ISocketChannel,public sharpen::Noncopyable
    {
    private:
        using MyImpl = sharpen::NativeSocketChannel;
        using MyImplPtr = std::unique_ptr<MyImpl>;
        
        MyImplPtr impl_;
    public:
    };
}

#endif
