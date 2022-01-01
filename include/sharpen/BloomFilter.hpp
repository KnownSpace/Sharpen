#pragma once
#ifndef _SHARPEN_BLOOMFILTER_HPP
#define _SHARPEN_BLOOMFILTER_HPP

#include <vector>
#include <functional>
#include <utility>
#include <cassert>

#include "TypeDef.hpp"

namespace sharpen
{
    template<typename _T>
    class BloomFilter
    {
    private:
        using Self = BloomFilter;
        using SpaceType = std::vector<char>;
        using Hasher = std::hash<_T>;

        SpaceType space_;
        sharpen::Size hashCount_;

        inline static sharpen::Size HashCode(const _T &obj) noexcept
        {
            return Hasher{}(obj);
        }
    public:
        BloomFilter(SpaceType space,sharpen::Size hashCount)
            :space_(std::move(space))
            ,hashCount_(hashCount)
        {}

        BloomFilter(sharpen::Size spaceSize,sharpen::Size hashCount)
            :space_()
            ,hashCount_(hashCount)
        {
            this->space_.assign(spaceSize,0);
        }
    
        BloomFilter(const Self &other)
            :space_(other.space_)
            ,hashCount_(other.hashCount_)
        {}
    
        BloomFilter(Self &&other) noexcept
            :space_(std::move(other.space_))
            ,hashCount_(other.hashCount_)
        {
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
                this->hashCount_ = other.hashCount_;
                other.hashCount_ = 0;
            }
            return *this;
        }

        void Add(const _T &obj) noexcept
        {
            assert(this->hashCount_ != 0);
            assert(this->space_.size() != 0);
            sharpen::Size hash = HashCode(obj);
            //double-hashing
            sharpen::Size delta = (hash >> 17) | (hash << 15);
            for (sharpen::Size i = 0; i < this->hashCount_; ++i)
            {
                sharpen::Size pos = hash % (this->space_.size()*8);
                this->space_[pos/8] |= (1 << (pos % 8));
                hash += delta;
            }
        }

        bool Containe(const _T &obj) noexcept
        {
            assert(this->hashCount_ != 0);
            assert(this->space_.size() != 0);
            sharpen::Size hash = HashCode(obj);
            //double-hashing
            sharpen::Size delta = (hash >> 17) | (hash << 15);
            for (sharpen::Size i = 0; i < this->hashCount_; ++i)
            {
                sharpen::Size pos = hash % (this->space_.size()*8);
                sharpen::Size bit = static_cast<sharpen::Size>(1) << (pos % 8);
                if(!(this->space_[pos/8] & bit))
                {
                    return false;
                }
                hash += delta;
            }
            return true;
        }

        SpaceType &Space() noexcept
        {
            return this->space_;
        }

        const SpaceType &Space() const noexcept
        {
            return this->space_;
        }
    
        ~BloomFilter() noexcept = default;
    };
}

#endif