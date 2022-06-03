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
#include <vector>

#include "SstKeyValuePair.hpp"
#include "IteratorOps.hpp"

namespace sharpen
{
    class SstKeyValueGroup
    {
    private:

        using Self = sharpen::SstKeyValueGroup;
        using Pairs = std::vector<sharpen::SstKeyValuePair>;
    public:

        using Iterator = typename Pairs::iterator;
        using ConstIterator = typename Pairs::const_iterator;
        using ReverseIterator = typename Pairs::reverse_iterator;
        using ConstReverseIterator = typename Pairs::const_reverse_iterator;
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
    
        Pairs pairs_;

        Comparator comp_;

        static bool Comp(const sharpen::SstKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        static bool WarppedComp(Comparator comp,const sharpen::SstKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        sharpen::Int32 CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        std::pair<sharpen::Uint64,sharpen::Uint64> ComputeKeySizes(const sharpen::ByteBuffer &key) const noexcept;
    public:
    
        SstKeyValueGroup();
    
        SstKeyValueGroup(const Self &other) = default;
    
        SstKeyValueGroup(Self &&other) noexcept = default;
    
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
                this->pairs_ = std::move(other.pairs_);
                this->comp_ = other.comp_;
                other.comp_ = nullptr;
            }
            return *this;
        }
    
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

        inline sharpen::ByteBuffer &GetValue(const sharpen::ByteBuffer &key)
        {
            return this->Get(key).Value();
        }

        inline const sharpen::ByteBuffer &GetValue(const sharpen::ByteBuffer &key) const
        {
            return this->Get(key).Value();
        }

        void Delete(const sharpen::ByteBuffer &key);

        inline Iterator Erase(Iterator where)
        {
            return this->pairs_.erase(where);
        }

        inline ConstIterator Erase(ConstIterator where)
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

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->pairs_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->pairs_.rbegin();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->pairs_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->pairs_.rend();
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

        inline void Reserve(sharpen::Size size)
        {
            this->pairs_.reserve(this->GetSize() + size);
        }

        inline void Resize(sharpen::Size size)
        {
            this->pairs_.resize(size);
        }

        inline sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key)
        {
            return this->Get(key).Value();
        }

        inline const sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key) const
        {
            return this->Get(key).Value();
        }

        inline sharpen::SstKeyValuePair &operator[](sharpen::Size index)
        {
            return this->pairs_.at(index);
        }

        inline const sharpen::SstKeyValuePair &operator[](sharpen::Size index) const
        {
            return this->pairs_.at(index);
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }

        inline void SetComparator(Comparator comp) noexcept
        {
            this->comp_ = comp;
        }
    };
}

#endif