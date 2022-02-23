#pragma once
#ifndef _SHARPEN_SORTEDSTRINGTABLE_HPP
#define _SHARPEN_SORTEDSTRINGTABLE_HPP

#include "SstRoot.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"
#include "ExistStatus.hpp"
#include "SortedStringTableBuilder.hpp"

namespace sharpen
{
    class SortedStringTable:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::SortedStringTable;

        sharpen::FileChannelPtr channel_;
        sharpen::SstRoot root_;
        sharpen::Size filterBits_;
        mutable sharpen::SegmentedCircleCache<sharpen::SstDataBlock> dataCache_;
        mutable sharpen::SegmentedCircleCache<sharpen::BloomFilter<sharpen::ByteBuffer>> filterCache_;

        static constexpr sharpen::Size blockSize_{4*1024};

        static constexpr sharpen::Size dataCacheSize_{512};

        static constexpr sharpen::Size filterCacheSize_{512};

        void LoadRoot();

        std::shared_ptr<sharpen::SstDataBlock> LoadDataBlockCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> LoadFilterCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

    public:
        //read
        explicit SortedStringTable(sharpen::FileChannelPtr channel);

        SortedStringTable(sharpen::FileChannelPtr channel,sharpen::Size dataCache,sharpen::Size filterCache);

        SortedStringTable(sharpen::FileChannelPtr channel,sharpen::Size bitsOfElements,sharpen::Size dataCache,sharpen::Size filterCache);

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size bitsOfElements,bool eraseDeleted,sharpen::Size dataCache,sharpen::Size filterCache)
            :channel_(std::move(channel))
            ,root_()
            ,filterBits_(bitsOfElements)
            ,dataCache_(dataCache)
            ,filterCache_(bitsOfElements != 0 ? filterCache:0)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            this->root_ = sharpen::SortedStringTableBuilder::DumpWalToTable<sharpen::SstDataBlock>(this->channel_,blockSize_,begin,end,bitsOfElements,eraseDeleted);
        }

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted,sharpen::Size dataCache,sharpen::Size filterCache)
            :SortedStringTable(std::move(channel),begin,end,10,eraseDeleted,dataCache,filterCache)
        {}

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted)
            :SortedStringTable(std::move(channel),begin,end,eraseDeleted,Self::dataCacheSize_,Self::filterCacheSize_)
        {}

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size bitsOfElements,bool eraseDeleted,bool ordered,sharpen::Size dataCache,sharpen::Size filterCache)
            :channel_(std::move(channel))
            ,root_()
            ,filterBits_(bitsOfElements)
            ,dataCache_(dataCache)
            ,filterCache_(bitsOfElements != 0 ? filterCache:0)
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
            }
            else
            {
                this->root_ = sharpen::SortedStringTableBuilder::MergeTables<sharpen::SstDataBlock>(this->channel_,Self::blockSize_,vec.begin(),vec.end(),filterBits_,eraseDeleted);
            }
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted,bool ordered,sharpen::Size dataCache,sharpen::Size filterCache)
            :SortedStringTable(std::move(channel),begin,end,10,eraseDeleted,ordered,dataCache,filterCache)
        {}

        template<typename _Iterator,typename _Check = decltype(std::declval<Self*&>() = &(*std::declval<_Iterator>()))>
        SortedStringTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted,bool ordered)
            :SortedStringTable(std::move(channel),begin,end,eraseDeleted,ordered,Self::dataCacheSize_,Self::filterCacheSize_)
        {}

        SortedStringTable(Self &&other) noexcept = default;
    
        ~SortedStringTable() noexcept = default;

        Self &operator=(Self &&other) noexcept;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        sharpen::SstDataBlock LoadDataBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::SstDataBlock LoadDataBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> QueryDataBlock(const sharpen::ByteBuffer &key) const;

        sharpen::ByteBuffer Query(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryQuery(const sharpen::ByteBuffer &key) const;

        bool Empty() const;

        const sharpen::SstRoot &Root() noexcept
        {
            return this->root_;
        }
    };
}

#endif