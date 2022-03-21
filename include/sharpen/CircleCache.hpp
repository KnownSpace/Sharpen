#pragma once
#ifndef _SHARPEN_CIRCLECACHE_HPP
#define _SHARPEN_CIRCLECACHE_HPP

#include <vector>
#include <utility>
#include <memory>
#include <mutex>
#include <cassert>
#include <string>

#include "TypeDef.hpp"
#include "SpinLock.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    //LRU Cache
    //second chances 
    template <typename _T>
    class CircleCache : public sharpen::Noncopyable, public sharpen::Nonmovable
    {
    private:
        struct CacheItem
        {
            std::vector<char> key_;
            std::shared_ptr<_T> cacheObj_;
            sharpen::Size chances_;
        };

        using Self = sharpen::CircleCache<_T>;
        using Buffer = std::vector<typename Self::CacheItem>;
        using Lock = sharpen::SpinLock;
        using Iterator = typename Buffer::iterator;
        using ConstIterator = typename Buffer::const_iterator;

        mutable Buffer buf_;
        mutable Lock lock_;
        sharpen::Size next_;

        template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
        Iterator Find(_Iterator keyBegin,_Iterator keyEnd) const noexcept
        {
            for (auto begin = this->buf_.begin(), end = this->buf_.end(); begin != end; ++begin)
            {
                if (begin->cacheObj_)
                {
                    bool match{true};
                    sharpen::Size index{0};
                    for (auto ite = keyBegin; ite != keyEnd; ++ite,++index)
                    {
                        if(*ite != begin->key_[index])
                        {
                            match = false;
                            break;
                        }
                    }
                    if(match)
                    {
                        return begin;
                    }
                }
            }
            return this->buf_.end();
        }

        template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
        std::shared_ptr<_T> UnlockedGet(_Iterator keyBegin,_Iterator keyEnd) const noexcept
        {
            std::shared_ptr<_T> result{nullptr};
            auto ite = this->Find(keyBegin,keyEnd);
            if(ite != this->buf_.end())
            {
                ite->chances_ = 1;
                result = ite->cacheObj_;
            }
            return result;
        }

        template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
        void UnlockedDelete(_Iterator keyBegin,_Iterator keyEnd) noexcept
        {
            auto ite = this->Find(keyBegin,keyEnd);
            if(ite != this->buf_.end())
            {
                ite->chances_ = 0;
                ite->cacheObj_.reset();
            }
        }
    public:
        explicit CircleCache(sharpen::Size size)
            :buf_(size)
            ,lock_()
            ,next_(0)
        {
            assert(size != 0);
        }

        ~CircleCache() noexcept = default;

        inline sharpen::Size GetSize() const noexcept
        {
            return this->buf_.size();
        }

        template <typename... _Args,typename _Check = decltype(new _T{std::declval<_Args>()...})>
        inline std::shared_ptr<_T> GetOrEmplace(const std::string &key, _Args &&...args)
        {
            return this->GetOrEmplace(key.begin(),key.end(),std::forward<_Args>(args)...);
        }

        template <typename _Iterator,typename... _Args,typename _Check = decltype(new _T{std::declval<_Args>()...},static_cast<char>(0) == *std::declval<_Iterator>())>
        inline std::shared_ptr<_T> GetOrEmplace(_Iterator begin,_Iterator end, _Args &&...args)
        {
            assert(begin != end);
            std::shared_ptr<_T> result{nullptr};
            {
                std::unique_lock<Lock> lock(this->lock_);
                result = this->UnlockedGet(begin,end);
                if(!result)
                {
                    result = std::make_shared<_T>(std::forward<_Args>(args)...);
                    CacheItem item;
                    item.key_.assign(begin,end);
                    item.cacheObj_ = result;
                    item.chances_ = 1;
                    sharpen::Size index = this->next_ % this->buf_.size();
                    while (this->buf_[index].chances_)
                    {
                        this->buf_[index].chances_ -= 1;
                        ++this->next_;
                        index = this->next_ % this->buf_.size();
                    }
                    this->buf_[index] = std::move(item);
                }
            }
            assert(result != nullptr);
            return result;
        }


        std::shared_ptr<_T> Get(const std::string &key) const noexcept
        {
            return this->Get(key.begin(),key.end());
        }

        template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
        std::shared_ptr<_T> Get(_Iterator begin,_Iterator end) const noexcept
        {
            assert(begin != end);
            std::unique_lock<Lock> lock(this->lock_);
            return this->UnlockedGet(begin,end);
        }

        template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
        void Delete(_Iterator begin,_Iterator end) noexcept
        {
            assert(begin != end);
            std::unique_lock<Lock> lock(this->lock_);
            return this->UnlockedDelete(begin,end);
        }

        void Delete(const std::string &key) noexcept
        {
            return this->Delete(key.begin(),key.end());
        }
    };
}

#endif