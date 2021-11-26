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
        IInputPipeChannel() = default;

        IInputPipeChannel(const Self &other) = default;

        IInputPipeChannel(Self &&other) noexcept = default;

        virtual ~IInputPipeChannel() noexcept = default;

        template<typename ..._Args>
        int ScanfAsync(const char *format,_Args &&...args)
        {
            sharpen::ByteBuffer buf(16);
            sharpen::Size size = this->ReadAsync(buf);
            while (size == buf.GetSize())
            {
                buf.Extend(64);
                size += this->ReadAsync(buf,size);
            }
            if (buf.Back() != '\0')
            {
                buf.PushBack('\0');
            }
            return std::sscanf(buf.Data(),format,std::forward<_Args>(args)...);
        }

        int GetcharAsync();

        sharpen::Size GetsAsync(char *buf,sharpen::Size bufSize);

        std::string GetsAsync();
    };

    using InputPipeChannelPtr = std::shared_ptr<sharpen::IInputPipeChannel>;

    sharpen::InputPipeChannelPtr MakeStdinPipe();
}

#endif