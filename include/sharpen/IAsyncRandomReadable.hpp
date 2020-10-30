#pragma once
#ifndef _SHARPEN_IASYNCRANDOMREADABLE_HPP
#define _SHARPEN_IASYNCRANDOMREADABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IAsyncRandomReadable
    {
    private:
        
        using Self = sharpen::IAsyncRandomReadable;
    public:
        
        IAsyncRandomReadable() = default;
        
        IAsyncRandomReadable(const Self &) = default;
        
        IAsyncRandomReadable(Self &&) noexcept = default;
        
        virtual ~IAsyncRandomReadable() noexcept = default;
        
        virtual sharpen::Size ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset) = 0;
        
        virtual sharpen::Size ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset) = 0;
    };
}

#endif
