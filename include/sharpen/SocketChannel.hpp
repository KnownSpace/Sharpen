#pragma once
#ifndef _SHARPEN_SOCKETCHANNEL_HPP
#define _SHARPEN_SOCKETCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class SocketChannel:public sharpen::IChannel,public sharpen::Noncopyable
    {
    private:
        
        sharpen::FileHandle handle_;
    public:
    };
}

#endif
