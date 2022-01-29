#pragma once
#ifndef _SHARPEN_PERSISTENTTABLE_HPP
#define _SHARPEN_PERSISTENTTABLE_HPP

#include "SortedStringTable.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"
#include "AsyncReadWriteLock.hpp"
#include "ExistStatus.hpp"
#include "PersistentTableConcept.hpp"

namespace sharpen
{
    class PersistentTable:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::PersistentTable;

        template<typename _Iterator>
        struct Convertor
        {
            static void Convert(sharpen::SstDataBlock &block,_Iterator ite)
            {
                if(!ite->second.IsDeleted())
                {
                    block.Put(ite->first,ite->second.Value());
                }
                else
                {
                    block.Delete(ite->first);
                }
            }
        };

        template<typename _Iterator>
        struct Convertor<std::move_iterator<_Iterator>>
        {
            static void Convert(sharpen::SstDataBlock &block,std::move_iterator<_Iterator> ite)
            {
                if(!ite->second.IsDeleted())
                {
                    block.Put(std::move(ite->first),std::move(ite->second.Value()));
                }
                else
                {
                    block.Delete(std::move(ite->first));
                }
            }
        };

        sharpen::FileChannelPtr channel_;
        sharpen::SortedStringTable root_;
        mutable sharpen::SegmentedCircleCache<sharpen::SstDataBlock> dataCache_;
        mutable sharpen::SegmentedCircleCache<sharpen::BloomFilter<sharpen::ByteBuffer>> filterCache_;

        static constexpr sharpen::Size blockSize_{4*1024};

        void LoadRoot();

        sharpen::SstDataBlock LoadDataBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::SstDataBlock LoadDataBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::SstDataBlock> LoadDataBlockCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> LoadFilterCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const;

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        void BuildTable(_Iterator begin,_Iterator end,bool eraseDeleted)
        {
            //build data block
            std::vector<sharpen::SstDataBlock> blocks;
            std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> filters;
            blocks.reserve(10);
            sharpen::Size blockSize{0};
            while (begin != end)
            {
                if(blocks.empty() || blockSize > blockSize_)
                {
                    blockSize = 0;
                    blocks.emplace_back();
                }
                blockSize += begin->first.GetSize();
                blockSize += begin->second.Value().GetSize();
                Self::Convertor<_Iterator>::Convert(blocks.back(),begin);
                ++begin;
            }
            filters.reserve(blocks.size());
            this->root_.IndexBlock().Reserve(blocks.size());
            this->root_.MetaIndexBlock().Reserve(blocks.size());
            sharpen::ByteBuffer buf;
            sharpen::Uint64 offset{0};
            //foreach blocks
            for (auto blockBegin = blocks.begin(),blockEnd = blocks.end(); blockBegin != blockEnd; ++blockBegin)
            {
                if(eraseDeleted)
                {
                    blockBegin->EraseDeleted();
                }
                //build filter
                filters.emplace_back(blockBegin->ComputeKeyCount()*10,10);
                for (auto groupBegin = blockBegin->Begin(),groupEnd = blockBegin->End(); groupBegin != groupEnd; ++groupBegin)
                {
                    for (auto keyBegin = groupBegin->Begin(),keyEnd = groupBegin->End(); keyBegin != keyEnd; ++keyBegin)
                    {
                        filters.back().Add(keyBegin->GetKey());  
                    }
                }
                sharpen::ByteBuffer lastKey{blockBegin->LastKey()};
                sharpen::ByteBuffer lasyKeyCopy{blockBegin->LastKey()};
                //write data blocks
                sharpen::Size size{blockBegin->StoreTo(buf)};
                try
                {
                    this->channel_->WriteAsync(buf.Data(),size,offset);
                }
                catch(const std::exception&)
                {
                    this->channel_->Truncate();
                    throw;
                }
                //build index block
                sharpen::SstBlock block;
                block.offset_ = offset;
                block.size_ = size;
                offset += size;
                this->root_.IndexBlock().Put(std::move(lastKey),block);
                this->root_.MetaIndexBlock().Put(std::move(lasyKeyCopy),block);
            }
            sharpen::Size index{0};
            for (auto blockBegin = blocks.begin(),blockEnd = blocks.end(); blockBegin != blockEnd; ++blockBegin)
            {
                sharpen::ByteBuffer lastKey{blockBegin->LastKey()};
                //write filter
                sharpen::Size size{filters[index].GetSize()};
                buf.ExtendTo(size);
                filters[index].CopyTo(buf.Data(),buf.GetSize());
                try
                {
                    this->channel_->WriteAsync(buf.Data(),size,offset);
                }
                catch(const std::exception&)
                {
                    this->channel_->Truncate();
                    throw;
                }
                //build meta index
                this->root_.MetaIndexBlock()[index].Block().offset_ = offset;
                this->root_.MetaIndexBlock()[index].Block().size_ = size;
                offset += size;
                index++;
            }
            filters.clear();
            filters.shrink_to_fit();
            blocks.clear();
            blocks.shrink_to_fit();
            this->root_.StoreTo(this->channel_,offset);
        }
    public:
    
        explicit PersistentTable(sharpen::FileChannelPtr channel);

        template<typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value>>
        PersistentTable(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,bool eraseDeleted)
            :channel_(std::move(channel))
            ,root_()
            ,dataCache_(64)
            ,filterCache_(64)
        {
            sharpen::Uint64 size = this->channel_->GetFileSize();
            if(size)
            {
                this->channel_->Truncate();
            }
            this->BuildTable(begin,end,eraseDeleted);
        }
    
        ~PersistentTable() noexcept = default;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::SstDataBlock> QueryDataBlock(const sharpen::ByteBuffer &key) const;

        sharpen::ByteBuffer Query(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryQuery(const sharpen::ByteBuffer &key) const;

        bool Empty() const;
    };
}

#endif