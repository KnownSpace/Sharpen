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
            assert(this->Value().GetSize() == sizeof(sharpen::FilePointer));
            return *reinterpret_cast<sharpen::FilePointer*>(this->Value().Data());
        }

        inline const sharpen::FilePointer &ValueAsPointer() const noexcept
        {
            assert(this->Value().GetSize() == sizeof(sharpen::FilePointer));
            return *reinterpret_cast<const sharpen::FilePointer*>(this->Value().Data());
        }

        sharpen::Size ComputeSize() const noexcept;

        static sharpen::Size ComputeSize(const sharpen::ByteBuffer &key,const sharpen::ByteBuffer &value) noexcept;

        sharpen::Size UnsafeStoreTo(char *data) const;

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };
}

#endif