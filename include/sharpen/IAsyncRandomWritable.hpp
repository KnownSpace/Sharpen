#pragma once
#ifndef _SHARPEN_IASYNCRANDOMWRITABLE_HPP
#define _SHARPEN_IASYNCRANDOMWRITABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IAsyncRandomWritable
    {
    private:
    
        using Self = sharpen::IAsyncRandomWritable;
    public:
        
        IAsyncRandomWritable() = default;
        
        IAsyncRandomWritable(const Self &) = default;
        
        IAsyncRandomWritable(Self &&) noexcept = default;
        
        virtual ~IAsyncRandomWritable() noexcept = default;
        
        virtual sharpen::Size WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset) = 0;
        
        virtual sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Uint64 offset) = 0;
    };
}

#endif _SHARPEN_IASYNCRANDOMWRITABLE_HPP
