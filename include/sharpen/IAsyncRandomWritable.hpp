#pragma once
#ifndef _SHARPEN_IASYNCRANDOMWRITABLE_HPP
#define _SHARPEN_IASYNCRANDOMWRITABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"
#include "Future.hpp"

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
        
        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) = 0;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) = 0;

        sharpen::Size WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset);

        sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset);

        sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Uint64 offset);

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline sharpen::Size WriteObjectAsync(const _T &obj,sharpen::Uint64 offset)
        {
            return this->WriteAsync(reinterpret_cast<const char*>(&obj),sizeof(obj),offset);
        }
    };
}

#endif