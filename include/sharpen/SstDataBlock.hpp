#pragma once
#ifndef _SHARPEN_SSTDATABLOCK_HPP
#define _SHARPEN_SSTDATABLOCK_HPP

/*
Key Value Pair
+---------------+
| SharedKeySize | varint
+---------------+
| UniqueKeySize | varint
+---------------+
| UniqueKey     |
+---------------+
| ValueSize     | varint 
+---------------+
| Value         |
+---------------+

Key Value Group
+-----------------+
| Key Value Pair1 |
+-----------------+
|       ...       |
+-----------------+
| Key Value PairN |
+-----------------+

DataBlock                   about 4*1024 bytes
+-------------------------+
| Crc16                   | 2 bytes
+-------------------------+
| Number of Group         | varint
+-------------------------+
| Key Value Group Offset1 | relative offset - 8 bytes
+-------------------------+
|          ...            |
+-------------------------+
| Key Value Group OffsetN |
+-------------------------+
| Key Value Group1        |
+-------------------------+
|          ...            |
+-------------------------+
| Key Value GroupN        |
+-------------------------+
*/

#include <vector>

#include "SstBlockHandle.hpp"
#include "SstKeyValueGroup.hpp"
#include "DataCorruptionException.hpp"

namespace sharpen
{
    class SstDataBlock
    {
    private:

        using Self = SstDataBlock;
        using Groups = std::vector<sharpen::SstKeyValueGroup>;
    public:

        using Iterator = typename Groups::iterator;
        using ConstIterator = typename Groups::const_iterator;
        using ReverseIterator = typename Groups::reverse_iterator;
        using ConstReverseIterator = typename Groups::const_reverse_iterator;
    private:

        template<typename _Iterator>
        struct MergeHelper
        {
            template<typename _KeyIterator>
            static void Merge(Self &block,_KeyIterator kv)
            {
                sharpen::ByteBuffer value{kv->Value()};
                sharpen::ByteBuffer key{kv->GetKey()};
                block.Put(std::move(key), std::move(value));
            }

            static void Clear(Self &block) noexcept
            {
                //do nothing
            }
        };

        template<typename _Iterator>
        struct MergeHelper<std::move_iterator<_Iterator>>
        {
            template<typename _KeyIterator>
            static void Merge(Self &block,_KeyIterator kv)
            {
                sharpen::ByteBuffer value{std::move(kv->Value())};
                sharpen::ByteBuffer key{std::move(*kv).MoveKey()};
                block.Put(std::move(key), std::move(value));
            }

            static void Clear(Self &block) noexcept
            {
                block.Clear();
            }
        };

        static constexpr sharpen::Size maxKeyPerGroups_{16};

        Groups groups_;

        static bool Less(const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept;

        Iterator FindInsertGroup(const sharpen::ByteBuffer &key);

        ConstIterator FindInsertGroup(const sharpen::ByteBuffer &key) const;
    public:
    
        SstDataBlock() = default;
    
        SstDataBlock(const Self &other) = default;
    
        SstDataBlock(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~SstDataBlock() noexcept = default;

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

        ConstIterator FindGroup(const sharpen::ByteBuffer &key) const;

        inline ConstIterator Begin() const noexcept
        {
            return this->groups_.cbegin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->groups_.cend();
        }

        inline Iterator Begin() noexcept
        {
            return this->groups_.begin();
        }

        inline Iterator End() noexcept
        {
            return this->groups_.end();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->groups_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->groups_.rbegin();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->groups_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->groups_.rend();
        }

        inline bool Empty() const noexcept
        {
            return this->groups_.empty();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->groups_.size();
        }

        bool Exist(const sharpen::ByteBuffer &key) const;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key);

        const sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key) const;

        Iterator FindGroup(const sharpen::ByteBuffer &key);

        sharpen::Size ComputeKeyCount() const noexcept;

        inline sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key)
        {
            return this->Get(key);
        }

        inline const sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key) const
        {
            return this->Get(key);
        }

        inline sharpen::SstKeyValueGroup &operator[](sharpen::Size index)
        {
            return this->groups_.at(index);
        }

        inline const sharpen::SstKeyValueGroup &operator[](sharpen::Size index) const
        {
            return this->groups_.at(index);
        }

        void MergeWith(sharpen::SstDataBlock block,bool reserveCurrent);

        template<typename _Iterator,typename _Check = decltype(std::declval<Self&>() = *std::declval<_Iterator>())>
        void MergeWith(_Iterator begin,_Iterator end,bool reserveCurrent)
        {
            if(reserveCurrent)
            {
                Self block;
                while (begin != end)
                {
                    for (auto groupBegin = begin->Begin(),groupEnd = begin->End(); groupBegin != groupEnd;++groupBegin)
                    {
                        for (auto keyBegin = groupBegin->Begin(),keyEnd = groupBegin->End(); keyBegin != keyEnd; ++keyBegin)
                        {
                            Self::MergeHelper<_Iterator>::Merge(block,keyBegin);
                        }
                    }
                    Self::MergeHelper<_Iterator>::Clear(*begin);
                    ++begin;
                }
                for (auto groupBegin = this->Begin(),groupEnd = this->End(); groupBegin != groupEnd; ++groupBegin)
                {
                    for (auto keyBegin = groupBegin->Begin(),keyEnd = groupBegin->End(); keyBegin != keyEnd; ++keyBegin)
                    {
                        sharpen::ByteBuffer value{keyBegin->Value()};
                        sharpen::ByteBuffer key{keyBegin->GetKey()};
                        block.Put(std::move(key), std::move(value));
                    }
                }
                *this = std::move(block);
                return;
            }
            Self block{*this};
            while (begin != end)
            {
                for (auto groupBegin = begin->Begin(),groupEnd = begin->End(); groupBegin != groupEnd; ++groupBegin)
                {
                    for (auto keyBegin = groupBegin->Begin(),keyEnd = groupBegin->End(); keyBegin != keyEnd; ++keyBegin)
                    {
                        Self::MergeHelper<_Iterator>::Merge(block,keyBegin); 
                    }
                }
                Self::MergeHelper<_Iterator>::Clear(*begin);
                ++begin;
            }
            *this = std::move(block);
        }

        Self Merge(Self other,bool reserveCurrent) const
        {
            Self tmp{*this};
            tmp.MergeWith(other,reserveCurrent);
            return tmp;
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self&>() = *std::declval<_Iterator>())>
        Self Merge(_Iterator begin,_Iterator end,bool reserveCurrent) const
        {
            Self tmp{*this};
            tmp.MergeWith(begin,end,reserveCurrent);
            return tmp;
        }

        sharpen::SstDataBlock Split();

        bool IsAtomic() const noexcept;

        const sharpen::ByteBuffer &FirstKey() const
        {
            return this->Begin()->First().GetKey();
        }

        const sharpen::ByteBuffer &LastKey() const
        {
            return this->ReverseBegin()->First().GetKey();
        }

        inline void Clear() noexcept
        {
            this->groups_.clear();
        }

        bool IsOverlapped(const Self &other) const noexcept;
    };
}

#endif