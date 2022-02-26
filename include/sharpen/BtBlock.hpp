#pragma once
#ifndef _SHARPEN_BTBLOCK_HPP
#define _SHARPEN_BTBLOCK_HPP

/*
+--------------+
| Depth        | Varint
+--------------+
| Next         | 16 Bytes
+--------------+
| Prev         | 16 Bytes
+--------------+
| Record Count |  2 Bytes
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
        using Iterator = typename Pairs::iterator;
        using ConstIterator = typename Pairs::const_iterator;
        using ReverseIterator = typename Pairs::reverse_iterator;
        using ConstReverseIterator = typename Pairs::const_reverse_iterator;

        static constexpr sharpen::Size defaultMaxRecordCount_{32};

        //persisent status
        sharpen::Size depth_;
        sharpen::FilePointer next_;
        sharpen::FilePointer prev_;
        Pairs pairs_;
        //volatile status
        sharpen::Size blockSize_;
        sharpen::Size usedSize_;

        static bool Comp(const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept;

        Iterator BinaryFind(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator BinaryFind(const sharpen::ByteBuffer &key) const noexcept;
    public:

        enum class PutTage
        {
            Normal,
            Append,
            MotifyEnd
        };

        BtBlock();
    
        explicit BtBlock(sharpen::Size blockSize);

        BtBlock(sharpen::Size blockSize,sharpen::Size maxRecordCount);
    
        BtBlock(const Self &other) = default;
    
        BtBlock(Self &&other) noexcept = default;
    
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
                this->depth_ = other.depth_;
                this->next_ = std::move(other.next_);
                this->prev_ = std::move(other.prev_);
                this->pairs_ = std::move(other.pairs_);
                this->blockSize_ = other.blockSize_;
                this->usedSize_ = other.usedSize_;
                other.usedSize_ = sizeof(this->next_) + 1 + sizeof(sharpen::Uint16);
                other.depth_ = 0;
                other.blockSize_ = 0;
            }
            return *this;
        }
    
        ~BtBlock() noexcept = default;

        inline sharpen::Size GetDepth() const noexcept
        {
            return this->depth_;
        }

        inline void SetDepth(sharpen::Size depth) noexcept
        {
            this->depth_ = depth;
        }

        inline sharpen::Size GetBlockSize() const noexcept
        {
            return this->blockSize_;
        }

        inline void SetBlockSize(sharpen::Size blockSize) noexcept
        {
            this->blockSize_ = blockSize;
        }

        inline sharpen::Size GetUsedSize() const noexcept
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

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
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

        inline sharpen::Size GetSize() const noexcept
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

        inline sharpen::Size GetAppendPointer() const noexcept
        {
            return this->usedSize_;
        }

        inline sharpen::Size ComputeMotifyEndPointer() const noexcept
        {
            return this->usedSize_ - this->End()->ComputeSize();
        }

        sharpen::Size ComputeCounterPointer() const noexcept;

        sharpen::Size ComputeNextPointer() const noexcept;

        sharpen::Size ComputePrevPointer() const noexcept;

        static inline constexpr sharpen::Size GetCounterSize() noexcept
        {
            //we use 2 bytes
            return sizeof(sharpen::Uint16);
        }
    };
}

#endif