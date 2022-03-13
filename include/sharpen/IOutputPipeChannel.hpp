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
    };

    using OutputPipeChannelPtr = std::shared_ptr<sharpen::IOutputPipeChannel>;
}

#endif