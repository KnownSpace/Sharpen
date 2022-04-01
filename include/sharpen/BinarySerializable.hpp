#pragma once
#ifndef _SHARPEN_BINARYSERIALIZABLE_HPP
#define _SHARPEN_BINARYSERIALIZABLE_HPP

#include <stdexcept>

#include "TypeTraits.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    template<typename _Object>
    class BinarySerializable
    {
    private:
        using Self = sharpen::BinarySerializable<_Object>;

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static constexpr sharpen::Size InternalComputeSize(const _T &obj,...) noexcept
        {
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<sharpen::Size&>() = std::declval<const _T&>().ComputeSize())>
        static constexpr sharpen::Size InternalComputeSize(const _T &obj,int) noexcept
        {
            return obj.ComputeSize();
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static sharpen::Size InternalLoadFrom(_T &obj,const char *data,sharpen::Size size,...)
        {
            if(size < sizeof(obj))
            {
                throw std::invalid_argument("invalid buffer");
            }
            std::memcpy(&obj,data,sizeof(obj));
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<sharpen::Size&>() = std::declval<_T&>().LoadFrom(nullptr,0))>
        static sharpen::Size InternalLoadFrom(_T &obj,const char *data,sharpen::Size size,int)
        {
            return obj.LoadFrom(data,size);
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static sharpen::Size InternalUnsafeStoreTo(const _T &obj,char *data,...) noexcept
        {
            std::memcpy(data,&obj,sizeof(obj));
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<sharpen::Size&>() = std::declval<const _T&>().UnsafeStoreTo(nullptr))>
        static sharpen::Size InternalUnsafeStoreTo(const _T &obj,char *data,int) noexcept
        {
            return obj.UnsafeStoreTo(data);
        }
    protected:

        template<typename _T,typename _Check = decltype(Self::InternalComputeSize(std::declval<const _T&>(),0))>
        static constexpr sharpen::Size ComputeSize(const _T &obj) noexcept
        {
            return Self::InternalComputeSize(obj,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,0,0))>
        static sharpen::Size LoadFrom(_T &obj,const char *data,sharpen::Size size)
        {
            return Self::InternalLoadFrom(obj,data,size,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,0,0))>
        static sharpen::Size LoadFrom(_T &obj,const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            return Self::LoadFrom(obj,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _T,typename _Check = decltype(Self::InternalUnsafeStoreTo<_T>(std::declval<const _T&>(),nullptr,0))>
        static sharpen::Size UnsafeStoreTo(const _T &obj,char *data) noexcept
        {
            return Self::InternalUnsafeStoreTo(obj,data,0);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T &>(),nullptr) + Self::ComputeSize(std::declval<const _T&>()))>
        static sharpen::Size StoreTo(const _T &obj,char *data,sharpen::Size size)
        {
            sharpen::Size needSize{Self::ComputeSize(obj)};
            if(size < needSize)
            {
                throw std::invalid_argument("buffer too small");
            }
            return Self::UnsafeStoreTo(obj,data);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T &>(),nullptr) + Self::ComputeSize(std::declval<const _T&>()))>
        static sharpen::Size StoreTo(const _T &obj,sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            sharpen::Size needSize{Self::ComputeSize(obj)};
            sharpen::Size size{buf.GetSize() - offset};
            if(needSize > size)
            {
                buf.Extend(needSize - size);
            }
            return Self::UnsafeStoreTo(obj,buf.Data() + offset);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T &>(),nullptr) + Self::ComputeSize<_T>(std::declval<const _T&>()))>
        static sharpen::Size StoreTo(const _T &obj,sharpen::ByteBuffer &buf)
        {
            return Self::StoreTo(obj,buf,0);
        }
    public:
    
        BinarySerializable() noexcept = default;
    
        BinarySerializable(const Self &other) noexcept = default;
    
        BinarySerializable(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~BinarySerializable() noexcept = default;

        sharpen::Size ComputeSize() const noexcept
        {
            return Self::ComputeSize(*static_cast<const _Object*>(this));
        }

        sharpen::Size LoadFrom(const char *data,sharpen::Size size)
        {
            return Self::LoadFrom(*static_cast<_Object*>(this),data,size);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return Self::LoadFrom(*static_cast<_Object*>(this),buf,offset);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return Self::LoadFrom(*static_cast<_Object*>(this),buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept
        {
            return Self::UnsafeStoreTo(*static_cast<_Object*>(this),data);
        }

        sharpen::Size StoreTo(char *data,sharpen::Size size) const
        {
            return Self::StoreTo(*static_cast<const _Object*>(this),data,size);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
        {
            return Self::StoreTo(*static_cast<const _Object*>(this),buf,offset);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return Self::StoreTo(*static_cast<const _Object*>(this),buf);
        }
    };   
}

#endif