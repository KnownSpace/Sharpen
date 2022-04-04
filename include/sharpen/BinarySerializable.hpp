#pragma once
#ifndef _SHARPEN_BINARYSERIALIZABLE_HPP
#define _SHARPEN_BINARYSERIALIZABLE_HPP

#include <stdexcept>

#include "TypeTraits.hpp"
#include "ByteBuffer.hpp"
#include "Varint.hpp"
#include "IteratorOps.hpp"
#include "IntOps.hpp"
#include "DataCorruptionException.hpp"

namespace sharpen
{
    struct BinarySerializator
    {
    private:
        using Self = sharpen::BinarySerializator;

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static constexpr sharpen::Size InternalComputeSize(const _T &obj,...) noexcept
        {
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<sharpen::Size&>() = std::declval<const _T&>().ComputeSize())>
        static constexpr sharpen::Size InternalComputeSize(const _T &obj,int,...) noexcept
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
        static sharpen::Size InternalLoadFrom(_T &obj,const char *data,sharpen::Size size,int,...)
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
        static sharpen::Size InternalUnsafeStoreTo(const _T &obj,char *data,int,...) noexcept
        {
            return obj.UnsafeStoreTo(data);
        }

        //container apis
        //[container size][elements]
        template<typename _Container,typename _Check = decltype(std::declval<_Container&>().reserve(0))>
        static void PrepareContainer(_Container &container,sharpen::Size size,int)
        {
            container.reserve(size);
        }

        template<typename _Container>
        static void PrepareContainer(_Container &container,sharpen::Size size,...)
        {}

        template<typename _Container>
        static auto InternalGetBegin(_Container &&container,int) -> decltype(container.Begin())
        {
            return container.Begin();
        }

        template<typename _Container>
        static auto InternalGetBegin(_Container &&container,...) -> decltype(container.begin())
        {
            return container.begin();
        }

        template<typename _Container>
        static auto GetBegin(_Container &&container) -> decltype(Self::InternalGetBegin(container,0))
        {
            return Self::InternalGetBegin(container,0);
        }

        template<typename _Container>
        static auto InternalGetEnd(_Container &&container,int) -> decltype(container.End())
        {
            return container.End();
        }

        template<typename _Container>
        static auto InternalGetEnd(_Container &&container,...) -> decltype(container.end())
        {
            return container.end();
        }

        template<typename _Container>
        static auto GetEnd(_Container &&container) -> decltype(Self::InternalGetEnd(container,0))
        {
            return Self::InternalGetEnd(container,0);
        }

        template<typename _Container,typename _Check = decltype(Self::InternalComputeSize(*Self::GetBegin(std::declval<const _Container&>()),0,0))>
        static sharpen::Size InternalComputeSize(const _Container &container,int,int) noexcept
        {
            sharpen::Varuint64 builder{sharpen::GetRangeSize(container.begin(),container.end())};
            return builder.ComputeSize() + Self::ComputeRangeSize(container.begin(),container.end());
        }

        template<typename _Container,typename _Check = decltype(Self::InternalLoadFrom(*Self::GetBegin(std::declval<_Container&>())++,nullptr,0,0,0))>
        static sharpen::Size InternalLoadFrom(_Container &container,const char *data,sharpen::Size size,int,int)
        {
            using ValueType = typename std::remove_reference<decltype(container.front())>::type;
            sharpen::Size offset{0};
            sharpen::Varuint64 builder{data,size};
            sharpen::Size count{sharpen::IntCast<sharpen::Size>(builder.Get())};
            Self::PrepareContainer(container,count);
            offset += builder.ComputeSize();
            while (count)
            {
                if(size <= offset)
                {
                    throw sharpen::DataCorruptionException("container data corruption");
                }
                ValueType obj;
                offset += Self::LoadFrom(obj,data + offset,size - offset);
                container.push_back(std::move(obj));
                --count;
            }
            return offset;
        }

        template<typename _Container,typename _Check = decltype(Self::InternalUnsafeStoreTo(*Self::GetBegin(std::declval<const _Container&>())++,nullptr,0,0))>
        static sharpen::Size InternalUnsafeStoreTo(const _Container &container,char *data,int,int) noexcept
        {
            sharpen::Size offset{0};
            auto begin = Self::GetBegin(container),end = Self::GetEnd(container);
            sharpen::Varuint64 builder{sharpen::GetRangeSize(begin,end)};
            sharpen::Size size{builder.ComputeSize()};
            std::memcpy(data,builder.Data(),size);
            offset += size;
            for (;begin != end; ++begin)
            {
                offset += Self::UnsafeStoreTo(*begin,data + offset);
            }
            return offset;
        }
    public:

        //single object apis
        template<typename _T,typename _Check = decltype(Self::InternalComputeSize(std::declval<const _T&>(),0,0))>
        static constexpr sharpen::Size ComputeSize(const _T &obj) noexcept
        {
            return Self::InternalComputeSize(obj,0,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,0,0,0))>
        static sharpen::Size LoadFrom(_T &obj,const char *data,sharpen::Size size)
        {
            return Self::InternalLoadFrom(obj,data,size,0,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,0,0,0))>
        static sharpen::Size LoadFrom(_T &obj,const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            return Self::LoadFrom(obj,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,0,0,0))>
        static sharpen::Size LoadFrom(_T &obj,const sharpen::ByteBuffer &buf)
        {
            return Self::LoadFrom(obj,buf,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalUnsafeStoreTo<_T>(std::declval<const _T&>(),nullptr,0,0))>
        static sharpen::Size UnsafeStoreTo(const _T &obj,char *data) noexcept
        {
            return Self::InternalUnsafeStoreTo(obj,data,0,0);
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

        //range apis
        template<typename _Iterator,typename _Check = decltype(Self::InternalComputeSize(*std::declval<_Iterator&>()++,0,0))>
        static sharpen::Size ComputeRangeSize(_Iterator begin,_Iterator end) noexcept
        {
            sharpen::Size size{0};
            while (begin != end)
            {
                size += Self::ComputeSize(*begin);
                ++begin;
            }
            return size;
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0,0))>
        static sharpen::Size LoadRangeFrom(_Iterator begin,_Iterator end,const char *data,sharpen::Size size)
        {
            sharpen::Size offset{0};
            while (begin != end)
            {
                offset += Self::LoadFrom(*begin,data + offset,size - offset);
            }
            return offset;
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0,0))>
        static sharpen::Size LoadRangeFrom(_Iterator begin,_Iterator end,const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            return Self::LoadRangeFrom(begin,end,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0,0))>
        static sharpen::Size LoadRangeFrom(_Iterator begin,_Iterator end,const sharpen::ByteBuffer &buf)
        {
            return Self::LoadRangeFrom(begin,end,buf,0);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static sharpen::Size UnsafeStoreRangeTo(_Iterator begin,_Iterator end,char *data) noexcept
        {
            sharpen::Size offset{0};
            while (begin != end)
            {
                offset += Self::UnsafeStoreTo(*begin,data + offset);
            }
            return offset;
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static sharpen::Size StoreRangeTo(_Iterator begin,_Iterator end,char *data,sharpen::Size size)
        {
            sharpen::Size needSize{Self::ComputeRangeSize(begin,end)};
            if(size < needSize)
            {
                throw std::invalid_argument("buffer too small");
            }
            return Self::UnsafeStoreRangeTo(begin,end,data);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static sharpen::Size StoreRangeTo(_Iterator begin,_Iterator end,sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            sharpen::Size needSize{Self::ComputeRangeSize(begin,end)};
            sharpen::Size size{buf.GetSize() - offset};
            if(size < needSize)
            {
                buf.Extend(needSize - size);
            }
            return Self::UnsafeStoreRangeTo(begin,end,buf.Data() + offset);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static sharpen::Size StoreRangeTo(_Iterator begin,_Iterator end,sharpen::ByteBuffer &buf)
        {
            return Self::StoreRangeTo(begin,end,buf,0);
        }
    };
    

    template<typename _Object>
    class BinarySerializable
    {
    private:
        using Self = sharpen::BinarySerializable<_Object>;
        
    protected:
        using Serializator = sharpen::BinarySerializator;
        using Helper = Self::Serializator;
    public:
    
        BinarySerializable() noexcept = default;
    
        BinarySerializable(const Self &other) noexcept = default;
    
        BinarySerializable(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~BinarySerializable() noexcept = default;

        sharpen::Size ComputeSize() const noexcept
        {
            return Serializator::ComputeSize(*static_cast<const _Object*>(this));
        }

        sharpen::Size LoadFrom(const char *data,sharpen::Size size)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),data,size);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),buf,offset);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept
        {
            return Serializator::UnsafeStoreTo(*static_cast<_Object*>(this),data);
        }

        sharpen::Size StoreTo(char *data,sharpen::Size size) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),data,size);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),buf,offset);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),buf);
        }

        const sharpen::BinarySerializable<_Object> &Serialize() const noexcept
        {
            return *this;
        }

        sharpen::BinarySerializable<_Object> &Unserialize() noexcept
        {
            return *this;
        }
    };   
}

#endif