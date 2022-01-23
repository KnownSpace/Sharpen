#pragma once
#ifndef _SHARPEN_BLOOMFILTER_HPP
#define _SHARPEN_BLOOMFILTER_HPP

#include <memory>
#include <functional>
#include <utility>
#include <cassert>
#include <atomic>
#include <stdexcept>

#include "TypeDef.hpp"
#include "BufferOps.hpp"

namespace sharpen
{
    struct BloomFilterHasher
    {
    private:

        template<typename _T,typename _Check = decltype(std::hash<_T>{}(std::declval<const _T&>()))>
        inline static sharpen::Size InternalHash(const _T &obj,...)
        {
            return std::hash<_T>{}(obj);
        }

        template<typename _T,typename _Check = typename std::enable_if<std::is_trivial<_T>::value>::type>
        inline static sharpen::Size InternalHash(const _T &obj,int)
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

    template<typename _T>
    class BloomFilter
    {
    private:
        using Self = BloomFilter;

        std::unique_ptr<std::atomic_char[]> space_;
        sharpen::Size size_;
        sharpen::Size hashCount_;

        inline static sharpen::Size HashCode(const _T &obj) noexcept
        {
            return sharpen::BloomFilterHasher::Hash(obj);
        }
    public:
        BloomFilter(sharpen::Size bitsOfSpace,sharpen::Size hashCount)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(hashCount)
        {
            bitsOfSpace = bitsOfSpace/8 + ((bitsOfSpace % 8)? 1:0);
            this->space_.reset(new std::atomic_char[bitsOfSpace]);
            this->size_ = bitsOfSpace;
            for (sharpen::Size i = 0; i != bitsOfSpace; ++i)
            {
                this->space_[i] = 0;
            }
        }

        BloomFilter(const char *space,sharpen::Size size,sharpen::Size hashCount)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(hashCount)
        {
            this->space_.reset(new std::atomic_char[size]);
            this->size_ = size;
            for (sharpen::Size i = 0; i != size; ++i)
            {
                this->space_[i] = *space++;
            }
        }
    
        BloomFilter(const Self &other)
            :space_(nullptr)
            ,size_(0)
            ,hashCount_(other.hashCount_)
        {
            this->space_.reset(new std::atomic_char[other.GetSize()]);
            this->size_ = other.GetSize();
            for (sharpen::Size i = 0; i != this->GetSize(); ++i)
            {
                this->space_[i] = other.space_[i].load();
            }
        }
    
        BloomFilter(Self &&other) noexcept
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
            sharpen::Size hash = HashCode(obj);
            //double-hashing
            sharpen::Size delta = (hash >> 17) | (hash << 15);
            for (sharpen::Size i = 0; i < this->hashCount_; ++i)
            {
                sharpen::Size pos = hash % (this->GetSize()*8);
                this->space_[pos/8] |= (1 << (pos % 8));
                hash += delta;
            }
        }

        bool Containe(const _T &obj) noexcept
        {
            assert(this->hashCount_ != 0);
            assert(this->GetSize() != 0);
            sharpen::Size hash = HashCode(obj);
            //double-hashing
            sharpen::Size delta = (hash >> 17) | (hash << 15);
            for (sharpen::Size i = 0; i < this->hashCount_; ++i)
            {
                sharpen::Size pos = hash % (this->GetSize()*8);
                sharpen::Size bit = static_cast<sharpen::Size>(1) << (pos % 8);
                if(!(this->space_[pos/8] & bit))
                {
                    return false;
                }
                hash += delta;
            }
            return true;
        }

        void CopyTo(char *data,sharpen::Size size) const
        {
            if(size < this->space_.size())
            {
                throw std::invalid_argument("buffer too small");
            }
            for (sharpen::Size i = 0; i != this->GetSize(); ++i)
            {
                *data++ = this->space_[i].load();
            }
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->size_;
        }
    
        ~BloomFilter() noexcept = default;
    };
}

#endif