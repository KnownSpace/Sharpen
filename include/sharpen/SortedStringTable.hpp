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

#include "SstRoot.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"
#include "ExistStatus.hpp"
#include "SortedStringTableBuilder.hpp"
#include "SstOption.hpp"

namespace sharpen
{
    class SortedStringTable:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::SortedStringTable;

        static constexpr sharpen::Size defaultFilterBits{10};

        sharpen::FileChannelPtr channel_;
        sharpen::SstRoot root_;
        sharpen::Size filterBits_;
        mutable sharpen::SegmentedCircleCache<sharpen::SstDataBlock> dataCache_;
        mutable sharpen::SegmentedCircleCache<sharpen::BloomFilter<sharpen::ByteBuffer>> filterCache_;

        void LoadRoot();

        std::shared_ptr<sharpen::SstDataBlock> LoadBlockCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> LoadFilterCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

    public:
        //read
        explicit SortedStringTable(sharpen::FileChannelPtr channel);

        SortedStringTable(sharpen::FileChannelPtr channel,sharpen::SstOption opt)
            :SortedStringTable(std::move(channel),Self::defaultFilterBits,opt)
        {}

        SortedStringTable(sharpen::FileChannelPtr channel,sharpen::Size bitsOfElements,sharpen::SstOption opt);

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted)
            :SortedStringTable(std::move(channel),begin,end,Self::defaultFilterBits,eraseDeleted)
        {}

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted)
            :SortedStringTable(std::move(channel),begin,end,filterBits,eraseDeleted,sharpen::SstOption{})
        {}

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted,sharpen::SstOption opt)
            :channel_(std::move(channel))
            ,root_()
            ,filterBits_(filterBits)
            ,dataCache_(opt.GetDataCacheSize())
            ,filterCache_(filterBits != 0 ? opt.GetFilterCacheSize():0)
        {
            this->Rebuild(begin,end,opt.GetBlockSize(),eraseDeleted);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted,bool ordered)
            :SortedStringTable(std::move(channel),begin,end,Self::defaultFilterBits,eraseDeleted,ordered)
        {}

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size filtersBits,bool eraseDeleted,bool ordered)
            :SortedStringTable(std::move(channel),begin,end,filtersBits,eraseDeleted,ordered,sharpen::SstOption{})
        {}

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size filtersBits,bool eraseDeleted,bool ordered,sharpen::SstOption opt)
            :channel_(std::move(channel))
            ,root_()
            ,filterBits_(filtersBits)
            ,dataCache_(opt.GetDataCacheSize())
            ,filterCache_(filtersBits != 0 ? opt.GetFilterCacheSize():0)
        {
            this->Rebuild(begin,end,opt.GetBlockSize(),eraseDeleted,ordered);
        }

        SortedStringTable(Self &&other) noexcept = default;
    
        ~SortedStringTable() noexcept = default;

        Self &operator=(Self &&other) noexcept;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        sharpen::SstDataBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::SstDataBlock LoadBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> GetBlockFromCache(const sharpen::ByteBuffer &key) const;

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
        void Rebuild(_Iterator begin,_Iterator end,sharpen::Size blockSize,bool eraseDeleted)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            this->root_ = sharpen::SortedStringTableBuilder::DumpWalToTable<sharpen::SstDataBlock>(this->channel_,blockSize,begin,end,this->filterBits_,eraseDeleted);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        void Rebuild(_Iterator begin,_Iterator end,sharpen::Size blockSize,bool eraseDeleted,bool ordered)
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
                this->root_ = sharpen::SortedStringTableBuilder::CombineTables<sharpen::SstDataBlock>(this->channel_,vec.begin(),vec.end(),this->filterBits_,eraseDeleted);
                return;
            }
            this->root_ = sharpen::SortedStringTableBuilder::MergeTables<sharpen::SstDataBlock>(this->channel_,blockSize,vec.begin(),vec.end(),this->filterBits_,eraseDeleted);
        }
    };
}

#endif