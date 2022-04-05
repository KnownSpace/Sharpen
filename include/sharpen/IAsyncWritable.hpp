#pragma once
#ifndef _SHARPEN_IASYNCWRITABLE_HPP
#define _SHARPEN_IASYNCWRITABLE_HPP

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"
#include "Future.hpp"

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

        virtual void WriteAsync(const sharpen::Char *buf, sharpen::Size bufSize, sharpen::Future<sharpen::Size> &future) = 0;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf, sharpen::Size bufferOffset, sharpen::Future<sharpen::Size> &future) = 0;

        sharpen::Size WriteAsync(const sharpen::Char *buf, sharpen::Size bufSize);

        sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset);

        sharpen::Size WriteAsync(const sharpen::ByteBuffer &buf);

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline sharpen::Size WriteObjectAsync(const _T &obj)
        {
            return this->WriteAsync(reinterpret_cast<const char*>(&obj),sizeof(obj));
        }
    };
}

#endif
