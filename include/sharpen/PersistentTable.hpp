#pragma once
#ifndef _SHARPEN_PERSISTENTTABLE_HPP
#define _SHARPEN_PERSISTENTTABLE_HPP

#include "SortedStringTable.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"
#include "AsyncReadWriteLock.hpp"
#include "ExistStatus.hpp"
#include "SortedStringTableBuilder.hpp"

namespace sharpen
{
    class PersistentTable:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::PersistentTable;

        sharpen::FileChannelPtr channel_;
        sharpen::SortedStringTable root_;
        sharpen::Size filterBits_;
        mutable sharpen::SegmentedCircleCache<sharpen::SstDataBlock> dataCache_;
        mutable sharpen::SegmentedCircleCache<sharpen::BloomFilter<sharpen::ByteBuffer>> filterCache_;

        static constexpr sharpen::Size blockSize_{4*1024};

        void LoadRoot();

        sharpen::SstDataBlock LoadDataBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::SstDataBlock LoadDataBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::SstDataBlock> LoadDataBlockCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> LoadFilterCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

    public:
    
        explicit PersistentTable(sharpen::FileChannelPtr channel);

        PersistentTable(sharpen::FileChannelPtr channel,sharpen::Size bitsOfElements);

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        PersistentTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size bitsOfElements,bool eraseDeleted)
            :channel_(std::move(channel))
            ,root_()
            ,filterBits_(bitsOfElements)
            ,dataCache_(64)
            ,filterCache_(64)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            this->root_ = sharpen::SortedStringTableBuilder::BuildTable<sharpen::SstDataBlock>(this->channel_,blockSize_,begin,end,bitsOfElements,eraseDeleted);
        }

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        PersistentTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted)
            :PersistentTable(std::move(channel),begin,end,10,eraseDeleted)
        {}
    
        ~PersistentTable() noexcept = default;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> QueryDataBlock(const sharpen::ByteBuffer &key) const;

        sharpen::ByteBuffer Query(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryQuery(const sharpen::ByteBuffer &key) const;

        bool Empty() const;
    };
}

#endif