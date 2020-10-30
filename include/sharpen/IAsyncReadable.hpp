#pragma once
#ifndef _SHARPEN_IASYNCREADABLE_HPP
#define _SHARPEN_IASYNCREADABLE_HPP

#include "ByteBuffer.hpp"
#include "TypeDef.hpp"

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
        
        virtual sharpen::Size ReadAsync(sharpen::Char *buf,sharpen::Size bufSize) = 0;
        
        virtual sharpen::Size ReadAsync(sharpen::ByteBuffer &buf) = 0;
    };
}

#endif
