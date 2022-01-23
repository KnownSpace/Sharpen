#pragma once
#ifndef _SHARPEN_CIRCLECACHE_HPP
#define _SHARPEN_CIRCLECACHE_HPP

#include <vector>
#include <utility>
#include <memory>
#include <mutex>

#include "TypeDef.hpp"
#include "SpinLock.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    template<typename _T>
    class CircleCache:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = CircleCache;
        using Buffer = std::vector<std::shared_ptr<_T>>;
        using Lock = sharpen::SpinLock;
        using Iterator = typename Buffer::iterator_type;
        using ConstIterator = typename Buffer::const_iterator_type;

        Buffer buf_;
        mutable Lock lock_;
        std::atomic_size_t next_;
    public:
        explicit CircleCache(sharpen::Size size)
            :buf_(size)
            ,lock_()
            ,next_(0)
        {}
    
        ~CircleCache() noexcept = default;

        inline sharpen::Size GetSize() const noexcept
        {
            return this->buf_.size();
        }

        inline sharpen::Size GetUsedSize() const noexcept
        {
            return (this->next_ < this->GetSize())? this->next_ : this->GetSize();
        }

        template<typename ..._Args>
        inline auto Emplace(_Args &&...args) -> decltype(new _T{std::declval<_Args>()...})
        {
            sharpen::Size next{this->next_.fetch_add(1)};
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->buf_[next] = std::make_shared<_T>(std::forward<_Args>(args)...);
            }
        }

        template<typename _Fn,typename _Check = sharpen::EnableIf<sharpen::IsCallableReturned<bool,_Fn,const _T&>>>
        std::shared_ptr<_T> Get(_Fn &&cond) const
        {
            std::shared_ptr<_T> result{nullptr};
            {
                std::unique_lock<Lock> lock(this->lock_);
                for (sharpen::Size i = 0,count = this->GetSize(); i != count; ++i)
                {
                    if (this->buf_[i] && cond(this->buf_[i]))
                    {
                        result = this->buf_[i];
                        break;
                    }
                }
            }
            return result;
        }
    }; 
}

#endif