#pragma once
#ifndef _SHARPEN_IASYNCRANDOMREADABLE_HPP
#define _SHARPEN_IASYNCRANDOMREADABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"
#include "Future.hpp"

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
        
        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) = 0;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) = 0;

        sharpen::Size ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset);

        sharpen::Size ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset);
    };
}

#endif
