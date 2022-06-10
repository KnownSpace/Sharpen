#pragma once
#ifndef _SHARPEN_BLOOMFILTER_HPP
#define _SHARPEN_BLOOMFILTER_HPP

#include <memory>
#include <functional>
#include <utility>
#include <cassert>
#include <atomic>
#include <stdexcept>

#include <cstdint>
#include <cstddef>
#include "BufferOps.hpp"

namespace sharpen
{
    struct BloomFilterHasher
    {
    private:

        template<typename _T,typename _Check = decltype(std::hash<_T>{}(std::declval<const _T&>()))>
        inline static std::size_t InternalHash(const _T &obj,...)
        {
            return std::hash<_T>{}(obj);
        }

        template<typename _T,typename _Check = typename std::enable_if<std::is_trivial<_T>::value>::type>
        inline static std::size_t InternalHash(const _T &obj,int)
        {
            return sharpen::BufferHash(reinterpret_cast<const char*>(&obj),sizeof(obj));
        }
    public:

        template<typename _T>
        inline static auto Hash(const _T &obj) -> decltype(InternalHash(obj,0))
        {
            return InternalHash(obj,0);
        }
    };

    template<typename _T,typename _Element>
    class InternalBloomFilter
    {
    private:
        using Self = sharpen::InternalBloomFilter<_T,_Element>;

        std::unique_ptr<_Element[]> space_;
        std::size_t size_;
        std::size_t hashCount_;

        inline static std::size_t HashCode(const _T &obj) noexcept
        {
            return sharpen::BloomFilterHasher::Hash(obj);
        }
    public:
        InternalBloomFilter(std::size_t bitsOfSpace,std::size_t hashCount)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(hashCount)
        {
            bitsOfSpace = bitsOfSpace/8 + ((bitsOfSpace % 8)? 1:0);
            this->space_.reset(new _Element[bitsOfSpace]);
            this->size_ = bitsOfSpace;
            for (std::size_t i = 0; i != bitsOfSpace; ++i)
            {
                this->space_[i] = 0;
            }
        }

        InternalBloomFilter(const char *space,std::size_t size,std::size_t hashCount)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(hashCount)
        {
            this->space_.reset(new _Element[size]);
            this->size_ = size;
            for (std::size_t i = 0; i != size; ++i)
            {
                this->space_[i] = *space++;
            }
        }
    
        InternalBloomFilter(const Self &other)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(other.hashCount_)
        {
            if(other.GetSize())
            {
                this->space_.reset(new _Element[other.GetSize()]);
                this->size_ = other.GetSize();
                for (std::size_t i = 0; i != this->GetSize(); ++i)
                {
                    this->space_[i] = static_cast<char>(other.space_[i]);
                }
            }
        }
    
        InternalBloomFilter(Self &&other) noexcept
            :space_(std::move(other.space_))
            ,size_(other.size_)
            ,hashCount_(other.hashCount_)
        {
            other.size_ = 0;
            other.hashCount_ = 0;
        }
    
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
                this->space_ = std::move(other.space_);
                this->size_ = other.size_;
                this->hashCount_ = other.hashCount_;
                other.size_ = 0;
                other.hashCount_ = 0;
            }
            return *this;
        }

        void Add(const _T &obj) noexcept
        {
            assert(this->hashCount_ != 0);
            assert(this->GetSize() != 0);
            std::size_t hash = HashCode(obj);
            //double-hashing
            std::size_t delta = (hash >> 17) | (hash << 15);
            for (std::size_t i = 0; i < this->hashCount_; ++i)
            {
                std::size_t pos = hash % (this->GetSize()*8);
                this->space_[pos/8] |= (1 << (pos % 8));
                hash += delta;
            }
        }

        bool Containe(const _T &obj) noexcept
        {
            assert(this->hashCount_ != 0);
            assert(this->GetSize() != 0);
            std::size_t hash = HashCode(obj);
            //double-hashing
            std::size_t delta = (hash >> 17) | (hash << 15);
            for (std::size_t i = 0; i < this->hashCount_; ++i)
            {
                std::size_t pos = hash % (this->GetSize()*8);
                std::size_t bit = static_cast<std::size_t>(1) << (pos % 8);
                if(!(this->space_[pos/8] & bit))
                {
                    return false;
                }
                hash += delta;
            }
            return true;
        }

        void CopyTo(char *data,std::size_t size) const
        {
            if(size < this->GetSize())
            {
                throw std::invalid_argument("buffer too small");
            }
            for (std::size_t i = 0; i != this->GetSize(); ++i)
            {
                *data++ = static_cast<char>(this->space_[i]);
            }
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->size_;
        }
    
        ~InternalBloomFilter() noexcept = default;
    };

    template<typename _T>
    using BloomFilter = sharpen::InternalBloomFilter<_T,char>;

    template<typename _T>
    using AtomicBloomFilter = sharpen::InternalBloomFilter<_T,std::atomic_char>;
}

#endif