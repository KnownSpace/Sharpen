#pragma once
#ifndef _SHARPEN_SEGMENTEDCIRCLECACHE_HPP
#define _SHARPEN_SEGMENTEDCIRCLECACHE_HPP

#include "CircleCache.hpp"

namespace sharpen
{
    template<typename _T>
    class SegmentedCircleCache:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::SegmentedCircleCache<_T>;
    
        sharpen::Size size_;
        sharpen::CircleCache<_T> *caches_;

        constexpr static sharpen::Size cacheSize_{3};

        inline sharpen::Size HashKey(const std::string &key) const noexcept
        {
            sharpen::Size hash{std::hash<std::string>{}(key)};
            return hash % this->size_;
        }

        void Release() noexcept
        {
            for (sharpen::Size i = 0; i != this->size_; ++i)
            {
                this->caches_[i].~CircleCache<_T>();   
            }
            this->size_ = 0;
            this->caches_ = nullptr;
        }
    public:
        explicit SegmentedCircleCache(sharpen::Size cacheSize)
            :size_(0)
            ,caches_(nullptr)
        {
            cacheSize >>= cacheSize_;
            if(!cacheSize)
            {
                cacheSize = 1;
            }
            this->caches_ = reinterpret_cast<sharpen::CircleCache<_T>*>(std::calloc(cacheSize,sizeof(*this->caches_)));
            if(!this->caches_)
            {
                throw std::bad_alloc();
            }
            for (;this->size_ != cacheSize; this->size_++)
            {
                try
                {
                    new (this->caches_ + this->size_) sharpen::CircleCache<_T>{1 << cacheSize_};
                }
                catch(const std::exception&)
                {
                    this->Release();
                    throw;   
                }
            }
        }
    
        ~SegmentedCircleCache() noexcept
        {
            this->Release();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->size_ << cacheSize_;
        }

        inline std::shared_ptr<_T> Get(const std::string &key) const noexcept
        {
            assert(!key.empty());
            return this->caches_[this->HashKey(key)].Get(key);
        }

        template <typename... _Args,typename _Check = decltype(new _T{std::declval<_Args>()...})>
        inline std::shared_ptr<_T> GetOrEmplace(const std::string &key, _Args &&...args)
        {
            assert(!key.empty());
            return this->caches_[this->HashKey(key)].GetOrEmplace(key,std::forward<_Args>(args)...);
        }
    };
}

#endif