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
#include <algorithm>

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

        static bool Comp(const sharpen::SstKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        std::pair<sharpen::Uint64,sharpen::Uint64> ComputeKeySizes(const sharpen::ByteBuffer &key) const noexcept;
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

        sharpen::Size UnsafeStoreTo(char *data) const;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        bool TryPut(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        bool CheckKey(const sharpen::ByteBuffer &key) const noexcept;

        sharpen::SstKeyValuePair &Get(const sharpen::ByteBuffer &key);

        const sharpen::SstKeyValuePair &Get(const sharpen::ByteBuffer &key) const;

        void Delete(const sharpen::ByteBuffer &key);

        inline ConstIterator Delete(ConstIterator where)
        {
            return this->pairs_.erase(where);
        }
        
        Iterator Find(const sharpen::ByteBuffer &key);

        ConstIterator Find(const sharpen::ByteBuffer &key) const;

        bool Exist(const sharpen::ByteBuffer &key) const;

        inline bool Empty() const noexcept
        {
            return this->pairs_.empty();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->pairs_.cbegin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->pairs_.cend();
        }

        inline Iterator Begin() noexcept
        {
            return this->pairs_.begin();
        }

        inline Iterator End() noexcept
        {
            return this->pairs_.end();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->pairs_.size();
        }

        const sharpen::SstKeyValuePair &First() const
        {
            return this->pairs_.front();
        }

        const sharpen::SstKeyValuePair &Last() const
        {
            return this->pairs_.back();
        }
    };
}

#endif