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
#include "ExistStatus.hpp"
#include "TwoWayIterator.hpp"

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
        using TwoWayIterator = sharpen::TwoWayIterator<Iterator>;
        using ConstTwoWayIterator = sharpen::TwoWayIterator<ConstIterator>;
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
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

            static void Clear(Self &&block) noexcept
            {
                block.Clear();
            }
        };

        static constexpr std::size_t maxKeyPerGroups_{16};

        Groups groups_;

        Comparator comp_;

        static bool WarppedComp(Comparator comp,const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept;

        static bool Comp(const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept;

        std::int32_t CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        Iterator FindInsertGroup(const sharpen::ByteBuffer &key);

        ConstIterator FindInsertGroup(const sharpen::ByteBuffer &key) const;
    public:
    
        SstDataBlock();
    
        SstDataBlock(const Self &other) = default;
    
        SstDataBlock(Self &&other) noexcept = default;
    
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
                this->groups_ = std::move(other.groups_);
                this->comp_ = other.comp_;
                other.comp_ = nullptr;
            }
            return *this;
        }
    
        ~SstDataBlock() noexcept = default;

        void LoadFrom(const char *data,std::size_t size);

        void LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
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

        inline std::size_t GetSize() const noexcept
        {
            return this->groups_.size();
        }

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(sharpen::ByteBuffer key);

        sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key);

        const sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key) const;

        Iterator FindGroup(const sharpen::ByteBuffer &key);

        std::size_t ComputeKeyCount() const noexcept;

        inline sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key)
        {
            return this->Get(key);
        }

        inline const sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key) const
        {
            return this->Get(key);
        }

        inline sharpen::SstKeyValueGroup &operator[](std::size_t index)
        {
            return this->groups_.at(index);
        }

        inline const sharpen::SstKeyValueGroup &operator[](std::size_t index) const
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
            return this->ReverseBegin()->ReverseBegin()->GetKey();
        }

        inline void Clear() noexcept
        {
            this->groups_.clear();
        }

        bool IsOverlapped(const Self &other) const noexcept;

        void EraseDeleted() noexcept;

        inline bool Contain(const sharpen::ByteBuffer &key) const noexcept
        {
            return this->Exist(key) != sharpen::ExistStatus::NotExist;
        }

        inline TwoWayIterator TwoWayBegin() noexcept
        {
            if(this->Empty())
            {
                return TwoWayIterator{this->Begin(),this->End(),{}};
            }
            return TwoWayIterator{this->Begin(),this->End(),this->Begin()->Begin()};
        }

        inline ConstTwoWayIterator TwoWayBegin() const noexcept
        {
            if(this->Empty())
            {
                return ConstTwoWayIterator{this->Begin(),this->End(),{}};
            }
            return ConstTwoWayIterator{this->Begin(),this->End(),this->Begin()->Begin()};
        }

        inline TwoWayIterator TwoWayEnd()
        {
            return TwoWayIterator{this->End(),this->End(),{}};
        }

        inline ConstTwoWayIterator TwoWayEnd() const
        {
            return ConstTwoWayIterator{this->End(),this->End(),{}};
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