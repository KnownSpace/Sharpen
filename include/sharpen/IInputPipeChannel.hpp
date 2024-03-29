#pragma once
#ifndef _SHARPEN_IINPUTPIPECHANNEL_HPP
#define _SHARPEN_IINPUTPIPECHANNEL_HPP

#include "AwaitableFuture.hpp"   // IWYU pragma: keep
#include "ByteBuffer.hpp"        // IWYU pragma: keep
#include "IAsyncReadable.hpp"
#include "IChannel.hpp"
#include <cstdio>
#include <cstring>

namespace sharpen {
    class IInputPipeChannel
        : public sharpen::IChannel
        , public sharpen::IAsyncReadable {
    private:
        using Self = sharpen::IInputPipeChannel;

    public:
        IInputPipeChannel() noexcept = default;

        IInputPipeChannel(const Self &other) = default;

        IInputPipeChannel(Self &&other) noexcept = default;

        virtual ~IInputPipeChannel() noexcept = default;

        int GetcharAsync();

        std::size_t GetsAsync(char *buf, std::size_t bufSize);

        std::string GetsAsync();
    };

    using InputPipeChannelPtr = std::shared_ptr<sharpen::IInputPipeChannel>;

    sharpen::InputPipeChannelPtr OpenStdinPipe();
}   // namespace sharpen

#endif