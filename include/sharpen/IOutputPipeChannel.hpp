#pragma once
#ifndef _SHARPEN_IOUTPUTPIPECHANNEL_HPP
#define _SHARPEN_IOUTPUTPIPECHANNEL_HPP

#include <cstring>
#include <cstdio>

#include "IChannel.hpp"
#include "IAsyncWritable.hpp"
#include "AwaitableFuture.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IOutputPipeChannel:public sharpen::IChannel,public sharpen::IAsyncWritable
    {
    private:
        using Self = sharpen::IOutputPipeChannel;

    public:
        IOutputPipeChannel() = default;

        IOutputPipeChannel(const Self &) = default;

        IOutputPipeChannel(Self &&) noexcept = default;

        virtual ~IOutputPipeChannel() noexcept = default;

        template<typename ..._Args>
        sharpen::Size PrintfAsync(const char *format,_Args &&...args)
        {
            sharpen::Size size = std::snprintf(nullptr,0,format,std::forward<_Args>(args)...);
            if(!size)
            {
                return 0;
            }
            sharpen::ByteBuffer buf(size + 1);
            std::snprintf(buf.Data(),buf.GetSize(),format,std::forward<_Args>(args)...);
            return this->WriteAsync(buf);
        }
    };

    using OutputPipeChannelPtr = std::shared_ptr<sharpen::IOutputPipeChannel>;
}

#endif