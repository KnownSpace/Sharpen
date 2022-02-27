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

        constexpr static sharpen::Size blockSize_{4*1024};

        static sharpen::Uint64 ComputeBlockSize(const sharpen::BtBlock &block) noexcept;

        sharpen::FilePointer AllocMemory(sharpen::Uint64 size);

        void FreeMemory(sharpen::FilePointer pointer);

        void InitFreeArea();

        void InitRoot();

        void InitFile();

        void WriteRootPointer(sharpen::FilePointer pointer);

        sharpen::FilePointer WriteEndOfBlock(sharpen::BtBlock &block,sharpen::Uint64 offset,sharpen::FilePointer pointer);

        sharpen::FilePointer WriteBlock(sharpen::BtBlock &block,sharpen::FilePointer pointer);

        sharpen::FilePointer InsertRecord(sharpen::BtBlock &block,sharpen::ByteBuffer key,sharpen::ByteBuffer value,sharpen::FilePointer pointer,sharpen::Optional<sharpen::BtBlock> &splitedBlock);
    
        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::ByteBuffer &buf) const;

        std::vector<std::pair<sharpen::BtBlock,sharpen::FilePointer>> GetPath(const sharpen::ByteBuffer &key) const;

        void InsertToRoot(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void DeleteFromRoot(const sharpen::ByteBuffer &key);

        std::pair<sharpen::BtBlock,sharpen::FilePointer> LoadBlockAndPointer(const sharpen::ByteBuffer &key) const;
    public:
    
        explicit BalancedTable(sharpen::FileChannelPtr channel);

        BalancedTable(sharpen::FileChannelPtr channel,sharpen::Uint16 maxRecordOfBlock);
    
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

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        sharpen::ByteBuffer Get(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryGet(const sharpen::ByteBuffer &key) const;

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        sharpen::BtBlock LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const;

        sharpen::BtBlock LoadBlock(const sharpen::ByteBuffer &key) const;

        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::FilePointer>())>
        inline void RangeQuery(_InsertIterator inserter,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const
        {
            assert(beginKey <= endKey);
            //get block and pointer
            auto beginPair{this->LoadBlockAndPointer(beginKey)};
            if(!beginPair.second.offset_ || !beginPair.second.size_)
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
                if(next.offset_ && next.size_)
                {
                    this->channel_->ReadAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),next.offset_ + nextPointer);
                }
                *inserter++ = beginPointer;
                //set begin pointer
                beginPointer = next;
                //set next pointer
                next = pointer;
            }
            if(beginPointer.offset_ && beginPointer.size_)
            {
                *inserter++ = beginPointer;
            }
        }
    };
}

#endif