#pragma once
#ifndef _SHARPEN_BTKEYVALUEPAIR_HPP
#define _SHARPEN_BTKEYVALUEPAIR_HPP

/*
+------------+
| Key Size   | Varint
+------------+
| Value Size | Varint
+------------+
| Key        |
+------------+
| Value      |
+------------+
*/

#include <cassert>

#include "ByteBuffer.hpp"
#include "DataCorruptionException.hpp"
#include "FilePointer.hpp"

namespace sharpen
{
    class BtKeyValuePair
    {
    private:
        using Self = sharpen::BtKeyValuePair;
    
        sharpen::ByteBuffer key_;
        sharpen::ByteBuffer value_;
    public:
        BtKeyValuePair() = default;

        BtKeyValuePair(sharpen::ByteBuffer key,sharpen::ByteBuffer value);
    
        BtKeyValuePair(const Self &other) = default;
    
        BtKeyValuePair(Self &&other) noexcept = default;
    
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
                this->key_ = std::move(other.key_);
                this->value_ = std::move(other.value_);
            }
            return *this;
        }
    
        ~BtKeyValuePair() noexcept = default;

        inline const sharpen::ByteBuffer &GetKey() const noexcept
        {
            return this->key_;
        }

        inline sharpen::ByteBuffer &&MoveKey() && noexcept
        {
            return std::move(this->key_);
        }

        sharpen::ByteBuffer &Value() noexcept
        {
            return this->value_;
        }

        const sharpen::ByteBuffer &Value() const noexcept
        {
            return this->value_;
        }

        inline sharpen::FilePointer &ValueAsPointer() noexcept
        {
            return this->Value().As<sharpen::FilePointer>();
        }

        inline const sharpen::FilePointer &ValueAsPointer() const noexcept
        {
            return this->Value().As<const sharpen::FilePointer>();
        }

        inline bool MayBePointer() const noexcept
        {
            return this->Value().GetSize() == sizeof(sharpen::FilePointer);
        }

        std::size_t ComputeSize() const noexcept;

        static std::size_t ComputeSize(const sharpen::ByteBuffer &key,const sharpen::ByteBuffer &value) noexcept;

        std::size_t UnsafeStoreTo(char *data) const;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };
}

#endif