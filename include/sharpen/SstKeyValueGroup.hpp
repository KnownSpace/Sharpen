#pragma once
#ifndef _SHARPEN_SSTKEYVALUEGROUP_HPP
#define _SHARPEN_SSTKEYVALUEGROUP_HPP

/*
Key Value Group
+-----------------+
| Key Value Pair1 |
+-----------------+
|       ...       |
+-----------------+
| Key Value PairN |
+-----------------+
*/

#include <stdexcept>

#include "SstKeyValuePair.hpp"
#include "IteratorOps.hpp"

namespace sharpen
{
    class SstKeyValueGroup
    {
    private:
        using Self = sharpen::SstKeyValueGroup;
        using Pairs = std::vector<sharpen::SstKeyValuePair>;
        using Iterator = typename Pairs::iterator;
        using ConstIterator = typename Pairs::const_iterator;
    
        Pairs pairs_;

        void InternalStoreTo(char *data,sharpen::Size size) const;
    public:
    
        SstKeyValueGroup() = default;
    
        SstKeyValueGroup(const Self &other) = default;
    
        SstKeyValueGroup(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~SstKeyValueGroup() noexcept = default;

        void LoadFrom(const char *data,sharpen::Size size);

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        sharpen::Size ComputeSize() const noexcept;

        void StoreTo(char *data,sharpen::Size size) const;

        void StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline void StoreTo(sharpen::ByteBuffer &buf) const
        {
            this->StoreTo(buf,0);
        }

        void Put(sharpen::SstKeyValuePair pair);

        sharpen::SstKeyValuePair &Get(const sharpen::ByteBuffer &key);

        const sharpen::SstKeyValuePair &Get(const sharpen::ByteBuffer &key) const;

        void Delete(const sharpen::ByteBuffer &key);

        void Update(const sharpen::ByteBuffer &oldKey,sharpen::SstKeyValuePair pair);

        Iterator Find(const sharpen::ByteBuffer &key);

        ConstIterator Find(const sharpen::ByteBuffer &key) const;

        bool Exist(const sharpen::ByteBuffer &key) const;

        inline bool Empty() const noexcept
        {
            return this->pairs_.empty();
        }

        inline Iterator Begin() noexcept
        {
            return this->pairs_.begin();
        }

        inline Iterator End() noexcept
        {
            return this->pairs_.end();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->pairs_.cbegin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->pairs_.cend();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->pairs_.size();
        }
    };
}

#endif