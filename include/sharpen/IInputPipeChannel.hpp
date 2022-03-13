#pragma once
#ifndef _SHARPEN_IINPUTPIPECHANNEL_HPP
#define _SHARPEN_IINPUTPIPECHANNEL_HPP

#include <cstring>
#include <cstdio>

#include "IChannel.hpp"
#include "IAsyncReadable.hpp"
#include "AwaitableFuture.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IInputPipeChannel:public sharpen::IChannel,public sharpen::IAsyncReadable
    {
    private:
        using Self = sharpen::IInputPipeChannel;

    public:
        IInputPipeChannel() noexcept = default;

        IInputPipeChannel(const Self &other) = default;

        IInputPipeChannel(Self &&other) noexcept = default;

        virtual ~IInputPipeChannel() noexcept = default;

        int GetcharAsync();

        sharpen::Size GetsAsync(char *buf,sharpen::Size bufSize);

        std::string GetsAsync();
    };

    using InputPipeChannelPtr = std::shared_ptr<sharpen::IInputPipeChannel>;

    sharpen::InputPipeChannelPtr MakeStdinPipe();
}

#endif