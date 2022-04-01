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
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset,sharpen::Future<sharpen::Size> &future) = 0;

        sharpen::Size ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset);

        sharpen::Size ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset);

        sharpen::Size ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset);

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline void ReadObjectAsync(_T &obj,sharpen::Uint64 offset)
        {
            this->ReadAsync(reinterpret_cast<char*>(&obj),sizeof(obj),offset);
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline void ReadObjectAsync(sharpen::Uint64 offset)
        {
            char buf[sizeof(_T)];
            this->ReadAsync(buf,sizeof(buf),offset);
            return *reinterpret_cast<_T*>(buf);
        }
    };
}

#endif
