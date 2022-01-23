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
    template <typename _T>
    class CircleCache : public sharpen::Noncopyable, public sharpen::Nonmovable
    {
    private:
        struct CacheItem
        {
            std::string key_;
            std::shared_ptr<_T> cacheObj_;
        };

        using Self = sharpen::CircleCache<_T>;
        using Buffer = std::vector<typename Self::CacheItem>;
        using Lock = sharpen::SpinLock;

        Buffer buf_;
        mutable Lock lock_;
        sharpen::Size next_;

        std::shared_ptr<_T> UnlockedGet(const std::string &key) const noexcept
        {
            std::shared_ptr<_T> result{nullptr};
            for (auto begin = this->buf_.begin(), end = this->buf_.end(); begin != end; ++begin)
            {
                if ((*begin).cacheObj_ && (*begin).key_ == key)
                {
                    result = (*begin).cacheObj_;
                    break;
                }
            }
            return result;
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
            assert(!key.empty());
            std::shared_ptr<_T> result{nullptr};
            {
                std::unique_lock<Lock> lock(this->lock_);
                result = this->UnlockedGet(key);
                if (!result)
                {
                    result = std::make_shared<_T>(std::forward<_Args>(args)...);
                    CacheItem item;
                    item.key_ = key;
                    item.cacheObj_ = result;
                    this->buf_[this->next_++ % this->buf_.size()] = std::move(item);
                }
            }
            assert(result != nullptr);
            return result;
        }

        std::shared_ptr<_T> Get(const std::string &key) const noexcept
        {
            assert(!key.empty());
            std::unique_lock<Lock> lock(this->lock_);
            return this->UnlockedGet(key);
        }
    };
}

#endif