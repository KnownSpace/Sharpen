#pragma once
#ifndef _SHARPEN_BTBLOCK_HPP
#define _SHARPEN_BTBLOCK_HPP

/*
+--------------+
| Depth        | varint - current depth
+--------------+
| Next         | 16 bytes - point to next block
+--------------+
| Prev         | 16 bytes - point to prev block
+--------------+
| Record Count |  2 bytes - record count
+--------------+
| Record1      | 
+--------------+
|     ...      |
+--------------+
| RecordN      |
+--------------+
| Free Area    |
+--------------+
*/

#include <vector>

#include "FilePointer.hpp"
#include "BtKeyValuePair.hpp"
#include "DataCorruptionException.hpp"
#include "BufferOps.hpp"
#include "ExistStatus.hpp"

namespace sharpen
{
    class BtBlock
    {
    private:
        using Self = sharpen::BtBlock;
        using Pairs = std::vector<sharpen::BtKeyValuePair>;

    public:
        using Iterator = typename Pairs::iterator;
        using ConstIterator = typename Pairs::const_iterator;
        using ReverseIterator = typename Pairs::reverse_iterator;
        using ConstReverseIterator = typename Pairs::const_reverse_iterator;
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer &,const sharpen::ByteBuffer &);

    private:
        static constexpr std::size_t defaultMaxRecordCount_{32};

        //persisent status
        std::size_t depth_;
        sharpen::FilePointer next_;
        sharpen::FilePointer prev_;
        Pairs pairs_;
        //volatile status
        std::size_t blockSize_;
        std::size_t usedSize_;
        std::uint64_t switzzPointer_;
        //comparator
        Comparator comp_;

        static bool Comp(const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        static bool WarppedComp(Comparator comp,const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        std::int32_t CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;
    public:

        enum class PutTage
        {
            Normal,
            Append,
            MotifyEnd
        };

        BtBlock();
    
        explicit BtBlock(std::size_t blockSize);

        BtBlock(std::size_t blockSize,std::size_t maxRecordCount);
    
        BtBlock(const Self &other) = default;
    
        BtBlock(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~BtBlock() noexcept = default;

        inline std::size_t GetDepth() const noexcept
        {
            return this->depth_;
        }

        inline void SetDepth(std::size_t depth) noexcept
        {
            this->depth_ = depth;
        }

        inline std::size_t GetBlockSize() const noexcept
        {
            return this->blockSize_;
        }

        inline void SetBlockSize(std::size_t blockSize) noexcept
        {
            this->blockSize_ = blockSize;
        }

        inline std::size_t GetUsedSize() const noexcept
        {
            return this->usedSize_;
        }

        inline sharpen::FilePointer &Next() noexcept
        {
            return this->next_;
        }

        inline const sharpen::FilePointer &Next() const noexcept
        {
            return this->next_;
        }

        inline sharpen::FilePointer &Prev() noexcept
        {
            return this->prev_;
        }

        inline const sharpen::FilePointer &Prev() const noexcept
        {
            return this->prev_;
        }

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        void FuzzingDelete(const sharpen::ByteBuffer &key);

        sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key);

        const sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key) const;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        Self Split();

        void Combine(Self other);

        inline bool IsAtomic() const noexcept
        {
            return this->Empty() || this->GetSize() == 1;
        }

        inline bool Empty() const noexcept
        {
            return this->pairs_.empty();
        }

        Iterator BinaryFind(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator BinaryFind(const sharpen::ByteBuffer &key) const noexcept;

        Iterator FuzzingFind(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator FuzzingFind(const sharpen::ByteBuffer &key) const noexcept;

        Iterator Find(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator Find(const sharpen::ByteBuffer &key) const noexcept;

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

        inline std::size_t GetSize() const noexcept
        {
            return this->pairs_.size();
        }

        PutTage QueryPutTage(const sharpen::ByteBuffer &key) const noexcept;

        inline bool IsAppend(const sharpen::ByteBuffer &key) const noexcept
        {
            return this->QueryPutTage(key) == Self::PutTage::Append;
        }

        inline bool IsMotifyEnd(const sharpen::ByteBuffer &key) const noexcept
        {
            return this->QueryPutTage(key) == Self::PutTage::MotifyEnd;
        }

        inline bool IsOverflow() const noexcept
        {
            return this->usedSize_ > this->blockSize_;
        }

        inline std::size_t GetAppendPointer() const noexcept
        {
            return this->usedSize_;
        }

        inline std::size_t ComputeMotifyEndPointer() const noexcept
        {
            return this->usedSize_ - this->ReverseBegin()->ComputeSize();
        }

        std::size_t ComputeCounterPointer() const noexcept;

        std::size_t ComputeNextPointer() const noexcept;

        std::size_t ComputePrevPointer() const noexcept;

        static inline constexpr std::size_t GetCounterSize() noexcept
        {
            //we use 2 bytes
            return sizeof(std::uint16_t);
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }

        inline void SetComparator(Comparator comp) noexcept
        {
            this->comp_ = comp;
        }

        inline std::uint64_t GetSwitzzPointer() const noexcept
        {
            return this->switzzPointer_;
        }

        void SetSwitzzPointer(std::uint64_t pointer) noexcept
        {
            this->switzzPointer_ = pointer;
        }
    };
}

#endif