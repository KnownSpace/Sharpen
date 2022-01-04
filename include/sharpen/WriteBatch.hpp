#pragma once
#ifndef _SHARPEN_WRITEBATCH_HPP
#define _SHARPEN_WRITEBATCH_HPP

#include <list>

#include "ByteBuffer.hpp"

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
            const sharpen::ByteBuffer *key_;
            sharpen::ByteBuffer *value_;
        };
        
    private:
        
        using Self = sharpen::WriteBatch;
        using Actions = std::list<Action>;
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

        void Put(const sharpen::ByteBuffer &key,sharpen::ByteBuffer &&value);

        void Delete(const sharpen::ByteBuffer &key);

        inline ConstIterator Begin() const noexcept
        {
            return this->actions_.cbegin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->actions_.cend();
        }

        inline void Clear() noexcept
        {
            return this->actions_.clear();
        }
    };   
}

#endif