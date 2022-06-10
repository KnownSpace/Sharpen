#pragma once
#ifndef _SHARPEN_LEVELVIEWITEM_HPP
#define _SHARPEN_LEVELVIEWITEM_HPP

/*
+-----------------+
| Begin key size  | varint
+-----------------+
| Begin key       |
+-----------------+
| End key size    | varint
+-----------------+
| End key         |
+-----------------+
| Id              | varint
+-----------------+
*/

#include "ByteBuffer.hpp"
#include "DataCorruptionException.hpp"
#include "IntOps.hpp"

namespace sharpen
{
    class LevelViewItem
    {
    private:
        using Self = sharpen::LevelViewItem;
    
        sharpen::ByteBuffer beginKey_;
        sharpen::ByteBuffer endKey_;
        std::uint64_t id_;
    public:
    
        LevelViewItem()
            :beginKey_()
            ,endKey_()
            ,id_(0)
        {}

        LevelViewItem(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,std::uint64_t id)
            :beginKey_(std::move(beginKey))
            ,endKey_(std::move(endKey))
            ,id_(id)
        {}
    
        LevelViewItem(const Self &other) = default;
    
        LevelViewItem(Self &&other) noexcept = default;
    
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
                this->beginKey_ = std::move(other.beginKey_);
                this->endKey_ = std::move(other.endKey_);
                this->id_ = other.id_;
                other.id_ = 0;
            }
            return *this;
        }
    
        ~LevelViewItem() noexcept = default;

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

        inline sharpen::ByteBuffer &BeginKey() noexcept
        {
            return this->beginKey_;
        }

        inline const sharpen::ByteBuffer &BeginKey() const noexcept
        {
            return this->beginKey_;
        }

        inline sharpen::ByteBuffer &EndKey() noexcept
        {
            return this->endKey_;
        }

        inline const sharpen::ByteBuffer &EndKey() const noexcept
        {
            return this->endKey_;
        }

        inline std::uint64_t GetId() const noexcept
        {
            return this->id_;
        }

        inline void SetId(std::uint64_t id) noexcept
        {
            this->id_ = id;
        }
    };
}

#endif