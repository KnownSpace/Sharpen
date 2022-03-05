#pragma once
#ifndef _SHARPEN_BALANCEDTABLE_HPP
#define _SHARPEN_BALANCEDTABLE_HPP

/*
+-------------------+
| Free Area Pointer |
+-------------------+
| Root Pointer      |
+-------------------+
| Block1            |
+-------------------+
| BlockN            |
+-------------------+
*/

#include <cassert>

#include "BtBlock.hpp"
#include "IFileChannel.hpp"
#include "Optional.hpp"
#include "SegmentedCircleCache.hpp"
#include "BtOption.hpp"

namespace sharpen
{
    class BalancedTable:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::BalancedTable;
    
        sharpen::FileChannelPtr channel_;
        std::vector<sharpen::FilePointer> freeArea_;
        sharpen::Size maxRecordsOfBlock_;
        sharpen::BtBlock root_;
        sharpen::Uint64 offset_;
        sharpen::FilePointer rootPointer_;
        mutable sharpen::SegmentedCircleCache<sharpen::BtBlock> caches_;

        constexpr static sharpen::Size blockSize_{4*1024};

        static sharpen::Uint64 ComputeBlockSize(const sharpen::BtBlock &block) noexcept;

        sharpen::Int32 CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        sharpen::FilePointer AllocMemory(sharpen::Uint64 size);

        void FreeMemory(sharpen::FilePointer pointer);

        void InitFreeArea();

        void InitRoot();

        void InitFile();

        std::shared_ptr<sharpen::BtBlock> LoadCache(sharpen::FilePointer pointer) const;

        std::shared_ptr<sharpen::BtBlock> LoadCache(sharpen::FilePointer pointer,sharpen::ByteBuffer &buf) const;

        std::shared_ptr<sharpen::BtBlock> LoadFromCache(sharpen::FilePointer pointer) const;

        void DeleteFromCache(sharpen::FilePointer pointer);

        void WriteRootPointer(sharpen::FilePointer pointer);

        sharpen::FilePointer WriteBlock(sharpen::BtBlock &block,sharpen::FilePointer pointer);

        sharpen::FilePointer InsertRecord(sharpen::BtBlock &block,sharpen::ByteBuffer key,sharpen::ByteBuffer value,sharpen::FilePointer pointer,sharpen::Optional<sharpen::BtBlock> &splitedBlock);
    
        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::ByteBuffer &buf) const;

        std::vector<std::pair<std::shared_ptr<sharpen::BtBlock>,sharpen::FilePointer>> GetPath(const sharpen::ByteBuffer &key,bool doCache) const;

        inline std::vector<std::pair<std::shared_ptr<sharpen::BtBlock>,sharpen::FilePointer>> GetPath(const sharpen::ByteBuffer &key) const
        {
            return this->GetPath(key,true);
        }

        void InsertToRoot(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void DeleteFromRoot(const sharpen::ByteBuffer &key);

        std::pair<sharpen::BtBlock,sharpen::FilePointer> LoadBlockAndPointer(const sharpen::ByteBuffer &key) const;
    public:
    
        explicit BalancedTable(sharpen::FileChannelPtr channel);

        BalancedTable(sharpen::FileChannelPtr channel,const sharpen::BtOption &opt);
    
        BalancedTable(Self &&other) noexcept = default;
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->channel_ = std::move(other.channel_);
                this->freeArea_ = std::move(other.freeArea_);
            }
            return *this;
        }
    
        ~BalancedTable() noexcept = default;

        inline sharpen::Size GetDepth() const noexcept
        {
            return this->root_.GetDepth();
        }

        inline const sharpen::BtBlock &Root() const noexcept
        {
            return this->root_;
        }

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        sharpen::ByteBuffer Get(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryGet(const sharpen::ByteBuffer &key) const;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        inline sharpen::BtBlock LoadBlock(sharpen::FilePointer pointer) const
        {
            return this->LoadBlock(pointer.offset_,pointer.size_);
        }

        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BtBlock LoadBlock(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::BtBlock> FindBlockFromCache(const sharpen::ByteBuffer &key) const;

        std::shared_ptr<const sharpen::BtBlock> FindBlock(const sharpen::ByteBuffer &key,bool doCache) const;

        inline std::shared_ptr<const sharpen::BtBlock> FindBlock(const sharpen::ByteBuffer &key) const
        {
            return this->FindBlock(key,true);
        }

        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const
        {
            assert(beginKey <= endKey);
            //get block and pointer
            auto beginPair{this->LoadBlockAndPointer(beginKey)};
            if(!beginPair.second.offset_ || !beginPair.second.size_)
            {
                return;
            }
            assert(!beginPair.first.Empty());
            if (this->CompKey(beginPair.first.Begin()->GetKey(),beginKey) == 1)
            {
                return;
            }
            auto endPair{this->LoadBlockAndPointer(endKey)};
            auto beginPointer{beginPair.second};
            auto endPointer{endPair.second};
            //load next pointer
            sharpen::Uint64 nextPointer{beginPair.first.ComputeNextPointer()};
            sharpen::FilePointer next{beginPair.first.Next()};
            while (beginPointer.offset_ != endPointer.offset_)
            {
                //load pointer
                sharpen::FilePointer pointer;
                std::memset(&pointer,0,sizeof(pointer));
                if(next.size_)
                {
                    this->channel_->ReadAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),next.offset_ + nextPointer);
                }
                *inserter++ = beginPointer;
                //set begin pointer
                beginPointer = next;
                //set next pointer
                next = pointer;
            }
            if(beginPointer.size_)
            {
                *inserter++ = beginPointer;
            }
        }

        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter) const
        {
            sharpen::Size depth{this->GetDepth()};
            if(!depth)
            {
                *inserter++ = this->rootPointer_;
                return;
            }
            sharpen::BtBlock leaf{this->root_};
            sharpen::FilePointer prevPointer;
            for (sharpen::Size i = 0; i != depth; ++i)
            {
                auto it = leaf.Begin();
                assert(it != leaf.End());
                prevPointer = it->ValueAsPointer();
                leaf = this->LoadBlock(prevPointer);
            }
            *inserter++ = prevPointer;
            sharpen::FilePointer next{leaf.Next()};
            while (next.offset_)
            {  
                *inserter++ = next;
                this->channel_->ReadAsync(reinterpret_cast<char*>(&next),sizeof(next),next.offset_ + leaf.ComputeNextPointer());
            }
        }

        //if return true
        //we should rebuild the table
        //using full table range query
        //and BtBlock::Next() to iterate full table
        //put key & value to a new table
        bool IsFault() const;

        inline void Flush()
        {
            assert(this->channel_);
            this->channel_->Flush();
        }

        inline sharpen::Uint64 GetSize() const noexcept
        {
            return this->offset_;
        }
    };
}

#endif