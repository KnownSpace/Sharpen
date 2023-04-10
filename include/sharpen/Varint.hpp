#pragma once
#ifndef _SHARPEN_VARINT_HPP
#define _SHARPEN_VARINT_HPP

#include <cassert>

#include "ByteBuffer.hpp"
#include "Optional.hpp"
#include "IntOps.hpp"
#include "CorruptedDataError.hpp"

namespace sharpen
{
    template<typename _T>
    class Varint
    {
    private:
        using Self = sharpen::Varint<_T>;

        constexpr static std::size_t bytes_ = sizeof(_T)*8/7 + ((sizeof(_T)*8 % 7)?1:0);
        constexpr static unsigned char mask_ = static_cast<unsigned char>(~(1 << 7));
        constexpr static unsigned char signBit_ = static_cast<unsigned char>(~mask_);

        mutable sharpen::Optional<_T> cache_;
        char data_[Self::bytes_];
    public:
        Varint() noexcept
            :Varint(0)
        {}

        explicit Varint(_T intValue) noexcept
            :cache_(sharpen::EmptyOpt)
            ,data_()
        {
            this->Set(intValue);
        }

        explicit Varint(sharpen::ByteBuffer data) noexcept
            :cache_(sharpen::EmptyOpt)
            ,data_()
        {
            this->Set(data);
        }

        Varint(const char *data,std::size_t size) noexcept
            :cache_(sharpen::EmptyOpt)
            ,data_()
        {
            this->Set(data,size);
        }
    
        Varint(const Self &other) noexcept
            :cache_(other.cache_)
            ,data_()
        {
            std::memcpy(this->data_,other.data_,bytes_);
        }
    
        Varint(Self &&other) noexcept
            :cache_(std::move(other.cache_))
            ,data_()
        {
            std::memcpy(this->data_,other.data_,bytes_);
        }
    
        inline Self &operator=(const Self &other) noexcept
        {
            if (this != std::addressof(other))
            {
                this->cache_ = other.cache_;
                std::memcpy(this->data_,other.data_,bytes_);
            }
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->cache_ = std::move(other.cache_);
                std::memcpy(this->data_,other.data_,bytes_);
            }
            return *this;
        }

        inline Self &operator=(_T val) noexcept
        {
            this->Set(val);
            return *this;
        }
    
        ~Varint() noexcept = default;

        inline _T Get() const noexcept
        {
            if(!this->cache_.Exist())
            {
                using UnsignedValue = typename std::make_unsigned<_T>::type;
                UnsignedValue value{0};
                const char *ite = this->data_;
                std::size_t counter{0};
                while (*ite & signBit_)
                {
                    value |= static_cast<UnsignedValue>((*ite & mask_)) << 7*counter;
                    ite += 1;
                    counter += 1;
                }
                value  |= static_cast<UnsignedValue>((*ite & mask_)) << 7*counter;
                this->cache_.Construct(static_cast<_T>(value));
            }
            return this->cache_.Get();
        }

        inline void Set(_T value) noexcept
        {
            if(this->cache_.Exist() && this->cache_.Get() == value)
            {
                return;
            }
            this->cache_.Construct(value);
            using UnsignedValue = typename std::make_unsigned<_T>::type;
            UnsignedValue val{static_cast<UnsignedValue>(value)};
            unsigned char *ite = reinterpret_cast<unsigned char*>(this->data_);
            for (;val > mask_; ++ite)
            {
                *ite = static_cast<unsigned char>(val) | signBit_;
                val >>= 7;
            }
            *ite = static_cast<unsigned char>(val);
        }

        inline void Set(const sharpen::ByteBuffer &data) noexcept
        {
            this->Set(data.Data(),data.GetSize());
        }

        inline void Set(const char *data,std::size_t size) noexcept
        {
            this->cache_.Reset();
            size = (std::min)(size,sizeof(this->data_));
            std::memcpy(this->data_,data,size);
            this->data_[size - 1] &= mask_;
        }

        inline operator _T() const noexcept
        {
            return this->Get();
        }

        inline bool operator==(const Self &other) const noexcept
        {
            return this->Get() == other.Get();
        }

        inline bool operator!=(const Self &other) const noexcept
        {
            return this->Get() != other.Get();
        }

        inline bool operator>=(const Self &other) const noexcept
        {
            return this->Get() >= other.Get();
        }

        inline bool operator<=(const Self &other) const noexcept
        {
            return this->Get() <= other.Get();
        }

        inline bool operator>(const Self &other) const noexcept
        {
            return this->Get() > other.Get();
        }

        inline bool operator<(const Self &other) const noexcept
        {
            return this->Get() < other.Get();
        }

        const char *Data() const noexcept
        {
            return this->data_;
        }

        inline std::size_t ComputeSize() const noexcept
        {
            std::size_t size{1};
            const char *ite = this->data_;
            while (*ite & signBit_)
            {
                ++ite;
                ++size;
            }
            return size;
        }

        static constexpr std::size_t GetMaxSize() noexcept
        {
            return sizeof(_T)*8/7 + ((sizeof(_T)*8 % 7)?1:0);
        }

        inline std::size_t LoadFrom(const char *data,std::size_t size)
        {
            this->Set(data,size);
            std::size_t offset{this->ComputeSize()};
            //tail-check
            assert((data[offset - 1] & signBit_) == 0);
            if(data[offset - 1] & signBit_)
            {
                throw sharpen::CorruptedDataError{"corrupted varint"};
            }
            return offset;
        }

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
        {
            assert(buf.GetSize() >= offset);
            return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
        }

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        inline std::size_t UnsafeStoreTo(char *data) const noexcept
        {
            std::size_t size{1};
            const char *ite = this->data_;
            while (*ite & signBit_)
            {
                *data++ = *ite;
                ++ite;
                ++size;
            }
            *data++ = *ite;
            return size;
        }

        inline std::size_t StoreTo(char *data,std::size_t size) const
        {
            std::size_t needSize{this->ComputeSize()};
            if(size < needSize)
            {
                throw std::invalid_argument("buffer too small");
            }
            return this->UnsafeStoreTo(data);
        }

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
        {
            assert(buf.GetSize() >= offset);
            std::size_t size{buf.GetSize() - offset};
            std::size_t needSize{this->ComputeSize()};
            if(size < needSize)
            {
                buf.Extend(needSize - size);
            }
            return this->UnsafeStoreTo(buf.Data() + offset);
        }

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
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

    using Varint32 = sharpen::Varint<std::int32_t>;
    using Varuint32 = sharpen::Varint<std::uint32_t>;

    using Varint64 = sharpen::Varint<std::int64_t>;
    using Varuint64 = sharpen::Varint<std::uint64_t>;

    using Varsize = sharpen::Varint<std::size_t>;
}

#endif