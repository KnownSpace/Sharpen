#pragma once
#ifndef _SHARPEN_IASYNCWRITABLE_HPP
#define _SHARPEN_IASYNCWRITABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IAsyncWritable
    {
    private:
         using Self = sharpen::IAsyncWritable;
    public:
         IAsyncWritable() = default;
         
         IAsyncWritable(const Self &) = default;
         
         IAsyncWritable(Self &&) noexcept = default;
         
         virtual ~IAsyncWritable() = default;
         
         virtual sharpen::Size WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize) = 0;
         
         virtual sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf) = 0;
    };
}

#endif
