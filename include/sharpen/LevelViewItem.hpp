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
        sharpen::Uint64 id_;
    public:
    
        LevelViewItem()
            :beginKey_()
            ,endKey_()
            ,id_(0)
        {}

        LevelViewItem(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,sharpen::Uint64 id)
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

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            this->StoreTo(buf,0);
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

        inline sharpen::Uint64 GetId() const noexcept
        {
            return this->id_;
        }

        inline void SetId(sharpen::Uint64 id) noexcept
        {
            this->id_ = id;
        }
    };
}

#endif