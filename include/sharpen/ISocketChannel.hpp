#pragma once
#ifndef _SHARPEN_SOCKETCHANNEL_HPP
#define _SHARPEN_SOCKETCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class ISocketChannel:public sharpen::IChannel,public sharpen::IAsyncWritable,public sharpen::IAsyncReadable
    {
    private:
        using Self = sharpen::ISocketChannel;
    public:
        
        ISocketChannel() = default;
        
        ~ISocketChannel() = default;
        
        ISocketChannel(const Self &other) = default;
        
        ISocketChannel(Self &&other) noexcept = default;
    };
}

#endif
