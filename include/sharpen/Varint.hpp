#pragma once
#ifndef _SHARPEN_VARINT_HPP
#define _SHARPEN_VARINT_HPP

#include <cassert>

#include "ByteBuffer.hpp"
#include "Optional.hpp"
#include "ByteOrder.hpp"
#include "IntOps.hpp"

namespace sharpen
{
    template<typename _T>
    class Varint
    {
    private:
        using Self = sharpen::Varint<_T>;

        constexpr static sharpen::Size bytes_ = sizeof(_T)*8/7 + ((sizeof(_T)*8 % 7)?1:0);
        constexpr static unsigned char mask_ = static_cast<unsigned char>(~(1 << 7));
        constexpr static unsigned char signBit_ = ~mask_;

        mutable sharpen::Optional<_T> cache_;
        sharpen::ByteBuffer data_;
    public:
        explicit Varint(_T intValue)
            :cache_(sharpen::EmptyOpt)
            ,data_()
        {
            this->Set(intValue);
        }

        explicit Varint(sharpen::ByteBuffer data)
            :cache_(sharpen::EmptyOpt)
            ,data_(std::move(data))
        {}

        Varint(const char *data,sharpen::Size size)
            :cache_(sharpen::EmptyOpt)
            ,data_(data,size)
        {}
    
        Varint(const Self &other)
            :cache_(other.cache_)
            ,data_(other.data_)
        {}
    
        Varint(Self &&other) noexcept
            :cache_(std::move(other.cache_))
            ,data_(std::move(other.data_))
        {}
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->cache_ = std::move(other.cache_);
                this->data_ = std::move(other.data_);
            }
            return *this;
        }
    
        ~Varint() noexcept = default;

        _T Get() const noexcept
        {
            if(!this->cache_.Exist())
            {
                _T value{0};
                if(!this->data_.Empty())
                {
                    if (this->data_.GetSize() == 1)
                    {
                        value = this->data_.Front() & mask_;
                        value <<= 1;
                    }
                    else
                    {
                        auto begin = this->data_.Begin(),end = this->data_.End();
                        for (; begin != end && (*begin & signBit_); ++begin)
                        {
                            value <<= 7;
                            value |= *begin & mask_;
                        }
                        sharpen::Size rawBytes = this->data_.GetSize()*7;
                        rawBytes /= 8;
                        assert(rawBytes <= sizeof(_T));
                        value <<= (rawBytes)*8 %7;
                        value |= static_cast<unsigned char>(*begin & mask_) >> (7 -  (rawBytes*8 % 7));
                        sharpen::ConvertEndian(reinterpret_cast<char*>(&value),rawBytes);
                    }
                }
                this->cache_.Construct(value);
            }
            return this->cache_.Get();
        }

        void Set(_T value)
        {
            if(this->cache_.Exist() && this->cache_.Get() == value)
            {
                return;
            }
            this->cache_.Construct(value);
            if(value == 0)
            {
                return;
            }
            this->data_.ExtendTo(bytes_);
            this->data_.Clear();
#ifdef SHARPEN_IS_LIL_ENDIAN
            const unsigned char *begin = reinterpret_cast<const unsigned char*>(&value);
            const unsigned char *end = begin + sizeof(_T);
#else
            const unsigned char *begin = reinterpret_cast<const unsigned char*>(&value) + sizeof(_T) - 1;
            const unsigned char *end = begin - 1;
#endif
            
            sharpen::Size offset{1};
            sharpen::Size min{sharpen::MinSizeof(value)};
            while (begin != end && min--)
            {
                if (offset == 1)
                {
                    this->data_.PushBack(*begin >> offset | signBit_);
                }
                else
                {
                    this->data_.Back() |= *begin >> offset;
                }
                unsigned char tmp = (*begin & ~(1 << offset));
                tmp <<= 7 - offset;
                if(min || tmp)
                {
                    this->data_.PushBack(tmp | signBit_);
                }
                if(++offset == 8)
                {
                    offset = 1;
                }
#ifdef SHARPEN_IS_LIL_ENDIAN
                ++begin;
#else
                --begin;
#endif
            }
            if(!this->data_.Empty())
            {
                this->data_.Back() &= mask_;
            }
        }

        inline operator _T() const noexcept
        {
            return this->Get();
        }

        inline bool operator==(const Self &other)
        {
            return this->Get() == other.Get();
        }

        inline bool operator!=(const Self &other)
        {
            return this->Get() != other.Get();
        }

        inline bool operator>=(const Self &other)
        {
            return this->Get() >= other.Get();
        }

        inline bool operator<=(const Self &other)
        {
            return this->Get() <= other.Get();
        }

        inline bool operator>(const Self &other)
        {
            return this->Get() > other.Get();
        }

        inline bool operator<(const Self &other)
        {
            return this->Get() < other.Get();
        }

        const sharpen::ByteBuffer &Data() const noexcept
        {
            return this->data_;
        }
    };

    template<typename _T>
    inline bool operator==(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() == obj;
    }

    template<typename _T>
    inline bool operator==(_T obj,const sharpen::Varint<_T> &varint)
    {
        return varint.Get() == obj;
    }

    template<typename _T>
    inline bool operator!=(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() != obj;
    }

    template<typename _T>
    inline bool operator!=(_T obj,const sharpen::Varint<_T> &varint)
    {
        return varint.Get() != obj;
    }

    template<typename _T>
    inline bool operator>=(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() >= obj;
    }

    template<typename _T>
    inline bool operator>=(_T obj,const sharpen::Varint<_T> &varint)
    {
        return obj >= varint.Get();
    }

    template<typename _T>
    inline bool operator>(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() > obj;
    }

    template<typename _T>
    inline bool operator>(_T obj,const sharpen::Varint<_T> &varint)
    {
        return obj > varint.Get();
    }

    template<typename _T>
    inline bool operator<=(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() <= obj;
    }

    template<typename _T>
    inline bool operator<=(_T obj,const sharpen::Varint<_T> &varint)
    {
        return obj <= varint.Get();
    }   

    template<typename _T>
    inline bool operator<(const sharpen::Varint<_T> &varint,_T obj)
    {
        return varint.Get() < obj;
    }

    template<typename _T>
    inline bool operator<(_T obj,const sharpen::Varint<_T> &varint)
    {
        return obj < varint.Get();
    }

    using Varint32 = sharpen::Varint<sharpen::Int32>;
    using Varuint32 = sharpen::Varint<sharpen::Uint32>;

    using Varint64 = sharpen::Varint<sharpen::Int64>;
    using Varuint64 = sharpen::Varint<sharpen::Uint64>;

    using Varsize = sharpen::Varint<sharpen::Size>;
}

#endif