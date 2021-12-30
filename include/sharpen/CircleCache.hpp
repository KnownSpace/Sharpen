#pragma once
#ifndef _SHARPEN_CIRCLECACHE_HPP
#define _SHARPEN_CIRCLECACHE_HPP

#include <vector>
#include <utility>

#include "TypeDef.hpp"
#include "Option.hpp"

namespace sharpen
{
    template<typename _T>
    class CircleCache
    {
    private:
        using Self = CircleCache;
        using Buffer = std::vector<sharpen::Option<_T>>;
        using Iterator = typename Buffer::iterator_type;
        using ConstIterator = typename Buffer::const_iterator_type;

        Buffer buf_;
        sharpen::Size next_;
    public:
        explicit CircleCache(sharpen::Size size)
            :buf_(size)
            ,next_(0)
        {}
    
        CircleCache(const Self &other)
            :buf_(other.buf_)
            ,next_(other.next_)
        {}
    
        CircleCache(Self &&other) noexcept
            :buf_(std::move(other.buf_))
            ,next_(other.next_)
        {
            other.next_ = 0;
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
                this->buf_ = std::move(other.buf_);
                this->next_ = other.next_;
                other.next_ = 0;
            }
            return *this;
        }
    
        ~CircleCache() noexcept = default;

        inline void Push(_T obj)
        {
            this->buf_[this->next_++ % this->buf_.size()].Construct(std::move(obj));
        }

        inline Iterator Begin()
        {
            return this->buf_.begin();
        }

        inline ConstIterator Begin() const
        {
            return this->buf_.begin();
        }

        inline Iterator End()
        {
            return this->buf_.end();
        }

        inline ConstIterator End() const
        {
            return this->buf_.end();
        }

        inline sharpen::Size Size() const noexcept
        {
            return this->buf_.size();
        }

        inline sharpen::Size UsedSize() const noexcept
        {
            return (this->next_ < this->Size())? this->next_ : this->Size();
        }

        template<typename ..._Args>
        inline auto Emplace(_Args &&...args) -> decltype(std::declval<sharpen::Option<_T>>().Construct(std::forward<_Args>(args)...))
        {
            this->buf_[this->next_++ % this->buf_.size()].Construct(std::forward<_Args>(args)...);
        }

        inline ReverseIterator ReverseBegin()
        {
            return this->vector_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const
        {
            return this->vector_.crbegin();
        }
        inline ReverseIterator ReverseEnd()
        {
            return this->vector_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const
        {
            return this->vector_.crend();
        } 
    }; 
}

#endif