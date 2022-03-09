#pragma once
#ifndef _SHARPEN_BALANCEDTABLE_HPP
#define _SHARPEN_BALANCEDTABLE_HPP

/*
+-------------------+
| Free Area Offset  | 8 bytes
+-------------------+
| Free Area Size    | 8 bytes - Point to the head of free areas list
+-------------------+
| Root Offset       | 8 bytes
+-------------------+
| Root Size         | 8 bytes Point to root node
+-------------------+
| Block1            |
+-------------------+
|       ....        |
+-------------------+
| BlockN            |
+-------------------+
| Free Areas(Opt)   |
+-------------------+
*/

#include <cassert>

#include "BtBlock.hpp"
#include "IFileChannel.hpp"
#include "Optional.hpp"
#include "SegmentedCircleCache.hpp"
#include "BtOption.hpp"
#include "LockTable.hpp"
#include "AsyncReadWriteLock.hpp"
#include "AsyncMutex.hpp"

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
        //file size
        sharpen::Uint64 offset_;
        //block cache
        mutable sharpen::SegmentedCircleCache<sharpen::BtBlock> caches_;
        //concurrency control
        mutable sharpen::LockTable<sharpen::Uint64,sharpen::AsyncReadWriteLock> lockTable_;
        mutable std::unique_ptr<sharpen::AsyncMutex> allocLock_;

        constexpr static sharpen::Size blockSize_{4*1024};

        sharpen::Uint64 ComputeBlockSize(const sharpen::BtBlock &block) const noexcept;

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

        sharpen::FilePointer AllocMemory(const sharpen::BtBlock &block);

        void FreeMemory(const sharpen::BtBlock &block);

        void WriteRootPointer(sharpen::FilePointer pointer);

        void WriteBlock(const sharpen::BtBlock &block);

        void AllocAndWriteBlock(sharpen::BtBlock &block);
    
        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::ByteBuffer &buf) const;

        std::vector<std::shared_ptr<sharpen::BtBlock>> GetPath(const sharpen::ByteBuffer &key,bool doCache) const;

        inline std::vector<std::shared_ptr<sharpen::BtBlock>> GetPath(const sharpen::ByteBuffer &key) const
        {
            return this->GetPath(key,true);
        }

        void DeleteFromRoot(const sharpen::ByteBuffer &key);
        void PutToRoot(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        static sharpen::FilePointer GetSwitzzPointer(const sharpen::BtBlock &block) noexcept;
    public:
    
        explicit BalancedTable(sharpen::FileChannelPtr channel);

        BalancedTable(sharpen::FileChannelPtr channel,const sharpen::BtOption &opt);
    
        BalancedTable(Self &&other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept;
    
        ~BalancedTable() noexcept = default;

        inline sharpen::Size GetDepth() const noexcept
        {
            return this->root_.GetDepth();
        }

        inline const sharpen::BtBlock &Root() const noexcept
        {
            return this->root_;
        }

        void LockBlockForWrite(const sharpen::BtBlock &block) const;

        void LockBlockForRead(const sharpen::BtBlock &block) const;

        void UnlockBlock(const sharpen::BtBlock &block) const noexcept;

        sharpen::AsyncReadWriteLock &GetRootLock() const;

        sharpen::AsyncReadWriteLock &GetBlockLock(const sharpen::BtBlock &block) const;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        sharpen::ByteBuffer Get(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryGet(const sharpen::ByteBuffer &key) const;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        inline sharpen::BtBlock LoadBlock(sharpen::FilePointer pointer) const
        {
            return this->LoadBlock(pointer.offset_,pointer.size_);
        }

        //unlocked
        //you should lock root lock(S) first
        //return a unique object of block
        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        //unlocked
        //you should lock root lock(S) first
        //return a unique object of block
        sharpen::BtBlock LoadBlock(const sharpen::ByteBuffer &key) const;

        //unlocked
        //you should lock root lock(S) first
        //and then lock block(S)
        std::shared_ptr<const sharpen::BtBlock> FindBlockFromCache(const sharpen::ByteBuffer &key) const;

        //unlocked
        //you should lock root lock(S) first
        //and then lock block(S)
        std::shared_ptr<const sharpen::BtBlock> FindBlock(const sharpen::ByteBuffer &key,bool doCache) const;

        //unlocked
        //you should lock root lock(S) first
        //and then lock block(S)
        inline std::shared_ptr<const sharpen::BtBlock> FindBlock(const sharpen::ByteBuffer &key) const
        {
            return this->FindBlock(key,true);
        }

        //unlocked
        //you should lock root lock(S) first
        //then call TableScan
        //and lock each of blocks(S)
        //then you call load and read the blocks
        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const
        {
            assert(beginKey <= endKey);
            if (this->root_.Empty())
            {
                return;
            }
            //get block and pointer
            auto beginBlock{this->LoadBlock(beginKey)};
            assert(!beginBlock.Empty());
            if (this->CompKey(beginBlock.Begin()->GetKey(),beginKey) == 1)
            {
                return;
            }
            auto endBlock{this->LoadBlock(endKey)};
            auto beginPointer{this->GetSwitzzPointer(beginBlock)};
            auto endPointer{this->GetSwitzzPointer(endBlock)};
            //load next pointer
            sharpen::Uint64 nextPointer{beginBlock.ComputeNextPointer()};
            sharpen::FilePointer next{beginBlock.Next()};
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

        //unlocked
        //you should lock root lock(S) first
        //then call TableScan
        //and lock each of blocks(S)
        //then you call load and read the blocks
        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void TableScan(_InsertIterator inserter) const
        {
            sharpen::Size depth{this->GetDepth()};
            if(!depth)
            {
                sharpen::FilePointer pointer;
                pointer.size_ = this->root_.GetBlockSize();
                pointer.offset_ = this->root_.GetSwitzzPointer();
                *inserter++ = pointer;
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