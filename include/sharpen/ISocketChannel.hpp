#pragma once
#ifndef _SHARPEN_ISOCKETCHANNEL_HPP
#define _SHARPEN_ISOCKETCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "IAsyncReadable.hpp"
#include "IAsyncWritable.hpp"

namespace sharpen
{
    class ISocketChannel:public sharpen::IChannel,public sharpen::IAsyncWritable,public sharpen::IAsyncReadable
    {
    private:
        using Self = sharpen::ISocketChannel;
    public:
        
        ISocketChannel() = default;
        
        virtual ~ISocketChannel() = default;
        
        ISocketChannel(const Self &) = default;
        
        ISocketChannel(Self &&) noexcept = default;
    };
}

#endif
