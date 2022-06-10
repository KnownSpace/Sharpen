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
#include <vector>

#include <cstdint>
#include <cstddef>
#include "ByteBuffer.hpp"

namespace sharpen
{
    //one component per level
    class LevelComponent
    {
    private:
        using Self = sharpen::LevelComponent;
        using Views = std::vector<std::uint64_t>;
    public:

        using Iterator = typename Views::iterator;
        using ConstIterator = typename Views::const_iterator;
        using ReverseIterator = typename Views::reverse_iterator;
        using ConstReverseIterator = typename Views::const_reverse_iterator;
    private:

        static constexpr std::size_t defaultReserveSize{16};
    
        Views views_;
    public:
    
        LevelComponent();

        explicit LevelComponent(std::size_t reserveSize);
    
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

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t ComputeSize() const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        void Put(std::uint64_t id);

        void Delete(std::uint64_t id);

        inline void Clear() noexcept
        {
            this->views_.clear();
        }

        inline std::size_t GetSize() const noexcept
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