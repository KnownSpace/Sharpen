#pragma once
#ifndef _SHARPEN_WRITEBATCH_HPP
#define _SHARPEN_WRITEBATCH_HPP

/*
+----------------------+
| Crc16                | 2 bytes
+----------------------+
| Number of Operations | varint
+----------------------+
| Type1                | 1 byte
+----------------------+
| KeySize1             | varint
+----------------------+
| Key1                 |
+----------------------+
| ValueSize1(only Put) | varint
+----------------------+
| Value1(only Put)     |
+----------------------+
|        ...           |
+----------------------+
| TypeN                |
+----------------------+
| KeySizeN             |
+----------------------+
| KeyN                 |
+----------------------+
| ValueSizeN(only Put) |
+----------------------+
| ValueN(Only Put)     |
+----------------------+
*/

#include <vector>

#include "ByteBuffer.hpp"
#include "DataCorruptionException.hpp"

namespace sharpen
{
    class WriteBatch
    {
    public:
        enum class ActionType
        {
            Put,
            Delete
        };

        struct Action
        {
            ActionType type_;
            sharpen::ByteBuffer key_;
            sharpen::ByteBuffer value_;
        };
        
    private:
        
        using Self = sharpen::WriteBatch;
        using Actions = std::vector<Action>;
        using Iterator = typename Actions::iterator;
        using ConstIterator = typename Actions::const_iterator;

        Actions actions_;
    public:
        WriteBatch() = default;
    
        WriteBatch(const Self &other) = default;
    
        WriteBatch(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~WriteBatch() noexcept = default;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(sharpen::ByteBuffer key);

        inline ConstIterator Begin() const noexcept
        {
            return this->actions_.cbegin();
        }

        inline Iterator Begin() noexcept
        {
            return this->actions_.begin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->actions_.cend();
        }

        inline Iterator End() noexcept
        {
            return this->actions_.end();
        }

        inline void Clear() noexcept
        {
            return this->actions_.clear();
        }

        void LoadFrom(const char *data,sharpen::Size size);

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size UnsafeStoreTo(char *data) const;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };   
}

#endif