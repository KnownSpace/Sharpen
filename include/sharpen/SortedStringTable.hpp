#pragma once
#ifndef _SHARPEN_SORTEDSTRINGTABLE_HPP
#define _SHARPEN_SORTEDSTRINGTABLE_HPP

/*
Sorted String Table
+------------------+
|Data Blocks       |
+------------------+
|Filter Blocks     |
+------------------+
|Meta Index Block  | 
+------------------+
|Meta Index Chksum | 2 bytes - Crc16
+------------------+
|Index Block       | 
+------------------+
|Index Chksum      | 2 bytes - Crc16
+------------------+
|Footer Block      | 32 bytes
+------------------+

Footer Block
+----------------------------+
|Offset of Index Blocks      |  8 bytes
+----------------------------+
|Size of Index Blocks        |  8 bytes
+----------------------------+
|Offset of Meta Index Blocks |  8 bytes
+----------------------------+
|Size of Meta Index Blocks   |  8 bytes
+----------------------------+

Index Block
+---------------------+
|Data Block1 Key Size | 8 bytes
+---------------------+
|Data Block1 Key      |
+---------------------+
|Offset of Data Block1| 8 bytes
+---------------------+
|Size of Data Block1  | 8 bytes
+---------------------+
|        .....        |
+---------------------+
|Data BlockN Key Size | 8 bytes
+---------------------+
|Data BlockN Key      |
+---------------------+
|Offset of Data BlockN| 8 bytes
+---------------------+
|Size of Data BlockN  | 8 bytes
+---------------------+

Meta Index Block
+------------------------+
|Filter Block1 Key size  | 8 bytes
+------------------------+
|Filter Block1 Key       |
+------------------------+
|Offset of Filter Block1 | 8 bytes
+------------------------+
|Size of Filter Block1   | 8 bytes
+------------------------+
|         .....          |
+------------------------+
|Filter BlockN Key size  |
+------------------------+
|Filter BlockN Key       |
+------------------------+
|Offset of Filter BlockN | 8 bytes
+------------------------+
|Size of Filter BlockN   | 8 bytes
+------------------------+

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

#include <cassert>

#include "SstRoot.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"
#include "ExistStatus.hpp"
#include "SortedStringTableBuilder.hpp"
#include "SstOption.hpp"
#include "SstBuildOption.hpp"

namespace sharpen
{
    class SortedStringTable:public sharpen::Noncopyable
    {
    public:
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::SortedStringTable;

        static constexpr sharpen::Size defaultFilterBits{10};

        sharpen::FileChannelPtr channel_;
        sharpen::SstRoot root_;
        sharpen::Size filterBitsOfElement_;
        mutable sharpen::SegmentedCircleCache<sharpen::SstDataBlock> dataCache_;
        mutable sharpen::SegmentedCircleCache<sharpen::BloomFilter<sharpen::ByteBuffer>> filterCache_;
        Comparator comp_;

        sharpen::Int32 CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        void LoadRoot();

        std::shared_ptr<sharpen::SstDataBlock> LoadBlockCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> LoadFilterCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

    public:
        explicit SortedStringTable(sharpen::FileChannelPtr channel);

        SortedStringTable(sharpen::FileChannelPtr channel,const sharpen::SstOption &opt);

        SortedStringTable(Self &&other) noexcept = default;
    
        ~SortedStringTable() noexcept = default;

        Self &operator=(Self &&other) noexcept;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        sharpen::SstDataBlock LoadBlock(sharpen::FilePointer pointer) const;

        sharpen::SstDataBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::SstDataBlock LoadBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> FindBlockFromCache(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> FindBlock(const sharpen::ByteBuffer &key,bool doCache) const;

        inline std::shared_ptr<const sharpen::SstDataBlock> FindBlock(const sharpen::ByteBuffer &key) const
        {
            return this->FindBlock(key,true);
        }

        sharpen::ByteBuffer Get(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryGet(const sharpen::ByteBuffer &key) const;

        bool Empty() const;

        const sharpen::SstRoot &Root() noexcept
        {
            return this->root_;
        }

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        void Build(_Iterator begin,_Iterator end,const sharpen::SstBuildOption &opt)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            this->root_ = sharpen::SortedStringTableBuilder::DumpWalToTable<sharpen::SstDataBlock>(this->channel_,opt.GetBlockSize(),begin,end,opt.GetFilterBitsOfElement(),opt.IsEraseDeleted(),this->comp_);
            this->filterBitsOfElement_ = opt.GetFilterBitsOfElement();
            this->channel_->Flush();
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        void Merge(_Iterator begin,_Iterator end,const sharpen::SstBuildOption &opt,bool ordered)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            std::vector<sharpen::SstVector> vec;
            vec.reserve(sharpen::GetRangeSize(begin,end));
            for (sharpen::Size i = 0;begin != end; ++begin,++i)
            {
                vec.emplace_back(&begin->Root(),begin->channel_);
            }
            if (ordered)
            {
                this->root_ = sharpen::SortedStringTableBuilder::CombineTables<sharpen::SstDataBlock>(this->channel_,vec.begin(),vec.end(),opt.GetFilterBitsOfElement(),opt.IsEraseDeleted(),this->comp_);
                this->filterBitsOfElement_ = opt.GetFilterBitsOfElement();
                return;
            }
            this->filterBitsOfElement_ = opt.GetFilterBitsOfElement();
            this->root_ = sharpen::SortedStringTableBuilder::MergeTables<sharpen::SstDataBlock>(this->channel_,opt.GetBlockSize(),vec.begin(),vec.end(),opt.GetFilterBitsOfElement(),opt.IsEraseDeleted(),this->comp_);
            this->channel_->Flush();
        }

        template<typename _Iterator,typename _InsertIterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>())),typename _CheckInsertor = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::SstRoot&>())>
        static void Merge(std::function<sharpen::FileChannelPtr()> maker,_Iterator begin,_Iterator end,sharpen::Size blocksOfTable,Comparator comparator,const sharpen::SstBuildOption &opt,_InsertIterator inserter)
        {
            std::vector<sharpen::SstVector> vec;
            vec.reserve(sharpen::GetRangeSize(begin,end));
            for (sharpen::Size i = 0;begin != end; ++begin,++i)
            {
                vec.emplace_back(&begin->Root(),begin->channel_);
            }
            sharpen::SortedStringTableBuilder::MergeTables<sharpen::SstDataBlock>(std::move(maker),opt.GetBlockSize(),blocksOfTable,vec.begin(),vec.end(),opt.GetFilterBitsOfElement(),opt.IsEraseDeleted(),inserter,comparator);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        static void Merge(std::function<sharpen::FileChannelPtr()> maker,_Iterator begin,_Iterator end,sharpen::Size blocksOfTable,Comparator comparator,const sharpen::SstBuildOption &opt)
        {
            std::vector<sharpen::SstRoot> roots;
            roots.reserve(16);
            Self::Merge(std::move(maker),begin,end,blocksOfTable,comparator,opt,std::back_inserter(roots));
            static_cast<void>(roots);
        }

        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const
        {
            assert(beginKey <= endKey);
            //query index block
            auto begin = this->root_.IndexBlock().Find(beginKey);
            if(begin == this->root_.IndexBlock().End())
            {
                return;
            }
            auto end = this->root_.IndexBlock().Find(endKey);
            while (begin != end)
            {
                *inserter++ = begin->Block();
                ++begin;
            }
            if (begin != this->root_.IndexBlock().End())
            {
                *inserter++ = begin->Block();
            }
        }

        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter) const
        {
            for (auto begin = this->root_.IndexBlock().Begin(),end = this->root_.IndexBlock().End(); begin != end; ++begin)
            {
                *inserter++ = begin->Block();   
            }
        }

        inline sharpen::Uint64 GetSize() const
        {
            return this->channel_->GetFileSize();
        }

        inline void Close() const
        {
            this->channel_->Close();
        }

        inline sharpen::Uint64 GetTableSize() const
        {
            return this->channel_->GetFileSize();
        }
    };
}

#endif