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

        inline sharpen::Size ReadFixedAsync(char *buf,std::size_t size)
        {
            sharpen::Size offset{0};
            while (offset != size)
            {
                sharpen::Size sz{this->ReadAsync(buf + offset,size - offset)};
                if(!sz)
                {
                    break;
                }   
                offset += sz;
            }
            return offset;
        }

        inline sharpen::Size ReadFixedAsync(sharpen::ByteBuffer &buf,sharpen::Size bufOffset)
        {
            assert(buf.GetSize() >= bufOffset);
            return this->ReadFixedAsync(buf.Data() + bufOffset,buf.GetSize() - bufOffset);
        }

        inline sharpen::Size ReadFixedAsync(sharpen::ByteBuffer &buf)
        {
            return this->ReadFixedAsync(buf,0);
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline sharpen::Size ReadObjectAsync(_T &obj)
        {
            return this->ReadFixedAsync(reinterpret_cast<char*>(&obj),sizeof(obj));
        }
    };
}

#endif
