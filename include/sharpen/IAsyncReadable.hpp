#pragma once
#ifndef _SHARPEN_IASYNCREADABLE_HPP
#define _SHARPEN_IASYNCREADABLE_HPP

#include "ByteBuffer.hpp"
#include <cstdint>
#include <cstddef>
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
        
        virtual void ReadAsync(char *buf,std::size_t bufSize,sharpen::Future<std::size_t> &future) = 0;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,std::size_t bufferOffset,sharpen::Future<std::size_t> &future) = 0;

        std::size_t ReadAsync(char *buf,std::size_t bufSize);

        std::size_t ReadAsync(sharpen::ByteBuffer &buf,std::size_t bufferOffset);

        std::size_t ReadAsync(sharpen::ByteBuffer &buf);

        inline std::size_t ReadFixedAsync(char *buf,std::size_t size)
        {
            std::size_t offset{0};
            while (offset != size)
            {
                std::size_t sz{this->ReadAsync(buf + offset,size - offset)};
                if(!sz)
                {
                    break;
                }   
                offset += sz;
            }
            return offset;
        }

        inline std::size_t ReadFixedAsync(sharpen::ByteBuffer &buf,std::size_t bufOffset)
        {
            assert(buf.GetSize() >= bufOffset);
            return this->ReadFixedAsync(buf.Data() + bufOffset,buf.GetSize() - bufOffset);
        }

        inline std::size_t ReadFixedAsync(sharpen::ByteBuffer &buf)
        {
            return this->ReadFixedAsync(buf,0);
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline std::size_t ReadObjectAsync(_T &obj)
        {
            return this->ReadFixedAsync(reinterpret_cast<char*>(&obj),sizeof(obj));
        }
    };
}

#endif
