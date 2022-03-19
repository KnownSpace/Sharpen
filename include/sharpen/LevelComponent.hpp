#pragma once
#ifndef _SHARPEN_LEVELCOMPONENT_HPP
#define _SHARPEN_LEVELCOMPONENT_HPP

/*
+-------------+
| View number | varint
+-------------+
| View 1 id   | varint
+-------------+
|    ...      |
+-------------+
| View N id   | varint
+-------------+ 
*/

#include <utility>

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    //one component per level
    class LevelComponent
    {
    private:
        using Self = sharpen::LevelComponent;
        using Views = std::vector<sharpen::Uint64>;
    public:

        using Iterator = typename Views::iterator;
        using ConstIterator = typename Views::const_iterator;
        using ReverseIterator = typename Views::reverse_iterator;
        using ConstReverseIterator = typename Views::const_reverse_iterator;
    private:

        static constexpr sharpen::Size defaultReserveSize{16};
    
        Views views_;
    public:
    
        LevelComponent();

        explicit LevelComponent(sharpen::Size reserveSize);
    
        LevelComponent(const Self &other) = default;
    
        LevelComponent(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->views_ = std::move(other.views_);
            }
            return *this;
        }
    
        ~LevelComponent() noexcept = default;

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        void Put(sharpen::Uint64 id);

        void Delete(sharpen::Uint64 id);

        inline void Clear() noexcept
        {
            this->views_.clear();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->views_.size();
        }

        inline bool Empty() const noexcept
        {
            return this->views_.empty();
        }

        inline Iterator Begin() noexcept
        {
            return this->views_.begin();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->views_.begin();
        }

        inline Iterator End() noexcept
        {
            return this->views_.end();
        }

        inline ConstIterator End() const noexcept
        {
            return this->views_.end();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->views_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->views_.rbegin();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->views_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->views_.rend();
        }
    };
}

#endif