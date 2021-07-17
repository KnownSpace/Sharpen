#pragma once
#ifndef _SHARPEN_IASYNCREADABLE_HPP
#define _SHARPEN_IASYNCREADABLE_HPP

#include "ByteBuffer.hpp"
#include "TypeDef.hpp"
#include "Future.hpp"

namespace sharpen
{

    class IAsyncReadable
    {
    private:
        using Self = sharpen::IAsyncReadable;
    public:
        IAsyncReadable() = default;
        
        IAsyncReadable(const Self &) = default;
        
        IAsyncReadable(Self &&) noexcept = default;
        
        virtual ~IAsyncReadable() noexcept = default;
        
        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) = 0;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) = 0;

        sharpen::Size ReadAsync(sharpen::Char *buf,sharpen::Size bufSize);

        sharpen::Size ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset);

        sharpen::Size ReadAsync(sharpen::ByteBuffer &buf);
    };
}

#endif
