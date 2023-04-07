#pragma once
#ifndef _SHARPEN_BINARYSERIALIZATOR_HPP
#define _SHARPEN_BINARYSERIALIZATOR_HPP

#include <stdexcept>
#include <vector>

#include "TypeTraits.hpp"
#include "ByteBuffer.hpp"
#include "Varint.hpp"
#include "IteratorOps.hpp"
#include "IntOps.hpp"
#include "Optional.hpp"
#include "CorruptedDataError.hpp"
#include "ByteOrder.hpp"

namespace sharpen
{
    //type should be default constructible
    struct BinarySerializator
    {
    private:
        using Self = sharpen::BinarySerializator;

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
        static void ToLittleEndian(const char *data,std::size_t size,int)
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::ConvertEndian(data,size);
#endif
        }

        template<typename _T>
        static void ToLittleEndian(const char *data,std::size_t size,...)
        {
            //do nothing
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static constexpr std::size_t InternalComputeSize(const _T &obj,...) noexcept
        {
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = std::declval<const _T&>().ComputeSize())>
        static std::size_t InternalComputeSize(const _T &obj,int,int,int,int,...) noexcept
        {
            return obj.ComputeSize();
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalComputeSize(std::declval<const _T&>(),0,0,0,0))>
        static std::size_t InternalComputeSize(const sharpen::Optional<_T> &obj,int,...) noexcept
        {
            std::size_t size{sizeof(bool)};
            if(obj.Exist())
            {
                size += Self::InternalComputeSize(obj.Get(),0,0,0,0);
            }
            return size;
        }

        template<typename _T1,typename _T2,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalComputeSize(std::declval<const _T1&>(),0,0,0,0) + Self::InternalComputeSize(std::declval<const _T2&>(),0,0,0,0))>
        static std::size_t InternalComputeSize(const std::pair<_T1,_T2> &obj,int,int,...) noexcept
        {
            std::size_t size{0};
            size += Self::ComputeSize(obj.first);
            size += Self::ComputeSize(obj.second);
            return size;
        }

        template<typename _Container,typename _Check = decltype(Self::InternalComputeSize(*sharpen::Begin(std::declval<const _Container&>()),0,0,0,0))>
        static std::size_t InternalComputeSize(const _Container &container,int,int,int,...) noexcept
        {
            std::size_t size{Self::ComputeRangeSize(sharpen::Begin(container),sharpen::End(container))};
            sharpen::Varuint64 builder{size};
            return builder.ComputeSize() + size;
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static std::size_t InternalLoadFrom(_T &obj,const char *data,std::size_t size,...)
        {
            if(size < sizeof(obj))
            {
                throw sharpen::CorruptedDataError("corrupted binary data");
            }
            {
                void *p{static_cast<void*>(&obj)};
                std::memcpy(p,data,sizeof(obj));
            }
#ifdef SHARPEN_IS_BIG_ENDIAN
            {
                char *p{reinterpret_cast<char*>(&p)};
                Self::ToLittleEndian(p,sizeof(obj),0);
            }
#endif
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = std::declval<_T&>().LoadFrom(nullptr,0))>
        static std::size_t InternalLoadFrom(_T &obj,const char *data,std::size_t size,int,int,int,int,...)
        {
            return obj.LoadFrom(data,size);
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalLoadFrom(std::declval<_T&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t InternalLoadFrom(sharpen::Optional<_T> &obj,const char *data,std::size_t size,int,...)
        {
            std::size_t offset{0};
            bool exist{false};
            std::memcpy(&exist,data,sizeof(exist));
            offset += sizeof(exist);
            if(exist)
            {
                if(size == offset)
                {
                    throw sharpen::CorruptedDataError("corrupted optional data");
                }
                //type should be default constructible
                obj.Construct();
                offset += Self::LoadFrom(obj.Get(),data + offset,size - offset);
            }
            else
            {
                obj.Reset();
            }
            return offset;
        }

        template<typename _T1,typename _T2,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalLoadFrom(std::declval<_T1&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0) + Self::InternalLoadFrom(std::declval<_T2&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t InternalLoadFrom(std::pair<_T1,_T2> &obj,const char *data,std::size_t size,int,int,...)
        {
            if(size < 2)
            {
                throw sharpen::CorruptedDataError("corrupted pair data");
            }
            std::size_t offset{0};
            offset += Self::LoadFrom(obj.first,data,size);
            if(size == offset)
            {
                throw sharpen::CorruptedDataError("corrupted pair data");
            }
            offset += Self::LoadFrom(obj.second,data + offset,size - offset);
            return offset;
        }

        template<typename _T>
        struct RemoveConst
        {
            using Type = typename std::remove_const<_T>::type;
        };

        template<typename _T1,typename _T2>
        struct RemoveConst<std::pair<_T1,_T2>>
        {
            using T1 = typename RemoveConst<_T1>::Type;
            using T2 = typename RemoveConst<_T2>::Type;
            using Type = std::pair<T1,T2>;
        };
        

        template<typename _Container,typename _Check = decltype(Self::InternalLoadFrom(*sharpen::Begin(std::declval<_Container&>())++,nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t InternalLoadFrom(_Container &container,const char *data,std::size_t size,int,int,int,...)
        {
            if(!size)
            {
                throw sharpen::CorruptedDataError("corrupted container data");
            }
            using ValueType = typename std::remove_reference<decltype(*sharpen::Begin(container))>::type;
            using NoconstValType = typename RemoveConst<ValueType>::Type;
            std::size_t offset{0};
            sharpen::Varuint64 builder{data,size};
            std::size_t count{sharpen::IntCast<std::size_t>(builder.Get())};
            Self::PrepareContainer(container,count);
            offset += builder.ComputeSize();
            while (count)
            {
                if(size <= offset)
                {
                    throw sharpen::CorruptedDataError("corrupted container data");
                }
                //type should be default constructible
                NoconstValType obj;
                offset += Self::LoadFrom(obj,data + offset,size - offset);
                Self::PutToContainer(container,std::move(obj));
                --count;
            }
            return offset;
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        static std::size_t InternalUnsafeStoreTo(const _T &obj,char *data,...) noexcept
        {
            std::memcpy(data,&obj,sizeof(obj));
#ifdef SHARPEN_IS_BIG_ENDIAN
            {
                Self::ToLiitleEndian(data,sizeof(obj),0);
            }
#endif
            return sizeof(obj);
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = std::declval<const _T&>().UnsafeStoreTo(nullptr))>
        static std::size_t InternalUnsafeStoreTo(const _T &obj,char *data,int,int,int,int,...) noexcept
        {
            return obj.UnsafeStoreTo(data);
        }

        template<typename _T,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalUnsafeStoreTo(std::declval<const _T&>(),nullptr,0,0,0,0))>
        static std::size_t InternalUnsafeStoreTo(const sharpen::Optional<_T> &obj,char *data,int,...) noexcept
        {
            std::size_t offset{0};
            bool exist{obj.Exist()};
            std::memcpy(data,&exist,sizeof(exist));
            offset += sizeof(exist);
            if(exist)
            {
                offset += Self::UnsafeStoreTo(obj.Get(),data + offset);
            }
            return offset;
        }

        template<typename _T1,typename _T2,typename _Check = decltype(std::declval<std::size_t&>() = Self::InternalUnsafeStoreTo(std::declval<const _T1&>(),nullptr,0,0,0,0) + Self::InternalUnsafeStoreTo(std::declval<const _T2&>(),nullptr,0,0,0,0))>
        static std::size_t InternalUnsafeStoreTo(const std::pair<_T1,_T2> &obj,char *data,int,int,...) noexcept
        {
            std::size_t offset{0};
            offset += Self::UnsafeStoreTo(obj.first,data);
            offset += Self::UnsafeStoreTo(obj.second,data + offset);
            return offset;
        }

        template<typename _Container,typename _Check = decltype(Self::InternalUnsafeStoreTo(*sharpen::Begin(std::declval<const _Container&>())++,nullptr,0,0,0,0))>
        static std::size_t InternalUnsafeStoreTo(const _Container &container,char *data,int,int,int,...) noexcept
        {
            std::size_t offset{0};
            auto begin = sharpen::Begin(container),end = sharpen::End(container);
            sharpen::Varuint64 builder{sharpen::GetRangeSize(begin,end)};
            std::size_t size{builder.ComputeSize()};
            std::memcpy(data,builder.Data(),size);
            offset += size;
            for (;begin != end; ++begin)
            {
                offset += Self::UnsafeStoreTo(*begin,data + offset);
            }
            return offset;
        }

        //container apis
        //[container size][elements]
        template<typename _Container,typename _Check = decltype(std::declval<_Container&>().reserve(0))>
        static void PrepareContainer(_Container &container,std::size_t size,int)
        {
            container.reserve(size);
        }

        template<typename _Container>
        static void PrepareContainer(_Container &container,std::size_t size,...)
        {
            static_cast<void>(container);
            static_cast<void>(size);
        }

        template<typename _Container,typename _T>
        static auto InternalPutToContainer(_Container &container,_T &&obj,int,int,int,int,...) -> decltype(container.Emplace(std::forward<_T>(obj)))
        {
            return container.Emplace(std::forward<_T>(obj));
        }

        template<typename _Container,typename _T>
        static auto InternalPutToContainer(_Container &container,_T &&obj,int,int,int,...) -> decltype(container.PushBack(std::forward<_T>(obj)))
        {
            return container.PushBack(std::forward<_T>(obj));
        }

        template<typename _Container,typename _T>
        static auto InternalPutToContainer(_Container &container,_T &&obj,int,int,...) -> decltype(container.emplace(std::forward<_T>(obj)))
        {
            return container.emplace(std::forward<_T>(obj));
        }

        template<typename _Container,typename _T>
        static auto InternalPutToContainer(_Container &container,_T &&obj,int,...) -> decltype(container.emplace_back(std::forward<_T>(obj)))
        {
            return container.emplace_back(std::forward<_T>(obj));
        }

        template<typename _Container,typename _T>
        static auto InternalPutToContainer(_Container &container,_T &&obj,...) -> decltype(container.push_back(std::forward<_T>(obj)))
        {
            return container.push_back(std::forward<_T>(obj));
        }

        template<typename _Container,typename _T>
        static auto PutToContainer(_Container &container,_T &&obj) -> decltype(Self::InternalPutToContainer(container,std::forward<_T>(obj),0,0,0,0))
        {
            return Self::InternalPutToContainer(container,std::forward<_T>(obj),0,0,0,0);
        }
    public:

        //single object apis
        template<typename _T,typename _Check = decltype(Self::InternalComputeSize(std::declval<const _T&>(),0,0,0,0))>
        static constexpr std::size_t ComputeSize(const _T &obj) noexcept
        {
            return Self::InternalComputeSize(obj,0,0,0,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t LoadFrom(_T &obj,const char *data,std::size_t size)
        {
            return Self::InternalLoadFrom(obj,data,size,0,0,0,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t LoadFrom(_T &obj,const sharpen::ByteBuffer &buf,std::size_t offset)
        {
            assert(buf.GetSize() >= offset);
            return Self::LoadFrom(obj,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _T,typename _Check = decltype(Self::InternalLoadFrom(std::declval<_T&>(),nullptr,static_cast<std::size_t>(0),0,0,0,0))>
        static std::size_t LoadFrom(_T &obj,const sharpen::ByteBuffer &buf)
        {
            return Self::LoadFrom(obj,buf,0);
        }

        template<typename _T,typename _Check = decltype(Self::InternalUnsafeStoreTo(std::declval<const _T&>(),nullptr,0,0,0,0))>
        static std::size_t UnsafeStoreTo(const _T &obj,char *data) noexcept
        {
            return Self::InternalUnsafeStoreTo(obj,data,0,0,0,0);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T&>(),nullptr) + Self::ComputeSize(std::declval<const _T&>()))>
        static std::size_t StoreTo(const _T &obj,char *data,std::size_t size)
        {
            std::size_t needSize{Self::ComputeSize(obj)};
            if(size < needSize)
            {
                throw std::invalid_argument("buffer too small");
            }
            return Self::UnsafeStoreTo(obj,data);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T&>(),nullptr) + Self::ComputeSize(std::declval<const _T&>()))>
        static std::size_t StoreTo(const _T &obj,sharpen::ByteBuffer &buf,std::size_t offset)
        {
            assert(buf.GetSize() >= offset);
            std::size_t needSize{Self::ComputeSize(obj)};
            std::size_t size{buf.GetSize() - offset};
            if(needSize > size)
            {
                buf.Extend(needSize - size);
            }
            return Self::UnsafeStoreTo(obj,buf.Data() + offset);
        }

        template<typename _T,typename _Check = decltype(Self::UnsafeStoreTo(std::declval<const _T&>(),nullptr) + Self::ComputeSize(std::declval<const _T&>()))>
        static std::size_t StoreTo(const _T &obj,sharpen::ByteBuffer &buf)
        {
            return Self::StoreTo(obj,buf,0);
        }

        //range apis
        template<typename _Iterator,typename _Check = decltype(Self::ComputeSize(*std::declval<_Iterator&>()++))>
        static std::size_t ComputeRangeSize(_Iterator begin,_Iterator end) noexcept
        {
            std::size_t size{0};
            while (begin != end)
            {
                size += Self::ComputeSize(*begin);
                ++begin;
            }
            return size;
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0))>
        static std::size_t LoadRangeFrom(_Iterator begin,_Iterator end,const char *data,std::size_t size)
        {
            std::size_t offset{0};
            while (begin != end)
            {
                offset += Self::LoadFrom(*begin,data + offset,size - offset);
            }
            return offset;
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0))>
        static std::size_t LoadRangeFrom(_Iterator begin,_Iterator end,const sharpen::ByteBuffer &buf,std::size_t offset)
        {
            assert(buf.GetSize() >= offset);
            return Self::LoadRangeFrom(begin,end,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _Iterator,typename _Check = decltype(Self::LoadFrom(*std::declval<_Iterator>()++,nullptr,0))>
        static std::size_t LoadRangeFrom(_Iterator begin,_Iterator end,const sharpen::ByteBuffer &buf)
        {
            return Self::LoadRangeFrom(begin,end,buf,0);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static std::size_t UnsafeStoreRangeTo(_Iterator begin,_Iterator end,char *data) noexcept
        {
            std::size_t offset{0};
            while (begin != end)
            {
                offset += Self::UnsafeStoreTo(*begin,data + offset);
            }
            return offset;
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static std::size_t StoreRangeTo(_Iterator begin,_Iterator end,char *data,std::size_t size)
        {
            std::size_t needSize{Self::ComputeRangeSize(begin,end)};
            if(size < needSize)
            {
                throw std::invalid_argument("buffer too small");
            }
            return Self::UnsafeStoreRangeTo(begin,end,data);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static std::size_t StoreRangeTo(_Iterator begin,_Iterator end,sharpen::ByteBuffer &buf,std::size_t offset)
        {
            assert(buf.GetSize() >= offset);
            std::size_t needSize{Self::ComputeRangeSize(begin,end)};
            std::size_t size{buf.GetSize() - offset};
            if(size < needSize)
            {
                buf.Extend(needSize - size);
            }
            return Self::UnsafeStoreRangeTo(begin,end,buf.Data() + offset);
        }

        template<typename _Iterator,typename _Check = decltype(Self::UnsafeStoreTo(*std::declval<_Iterator>()++,nullptr))>
        static std::size_t StoreRangeTo(_Iterator begin,_Iterator end,sharpen::ByteBuffer &buf)
        {
            return Self::StoreRangeTo(begin,end,buf,0);
        }
    };
}

#endif