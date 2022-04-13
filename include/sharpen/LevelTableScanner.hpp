#pragma once
#ifndef _SHARPEN_LEVELTABLESCANNER_HPP
#define _SHARPEN_LEVELTABLESCANNER_HPP

#include "MemoryTable.hpp"
#include "BinaryLogger.hpp"
#include "SortedStringTable.hpp"
#include "AsyncReadWriteLock.hpp"
#include "LevelComponent.hpp"
#include "MemoryTableComparator.hpp"
#include "LevelView.hpp"
#include "LevelTableOption.hpp"
#include "LockTable.hpp"

namespace sharpen
{
    class LevelTable;

    class LevelTableScanner:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::LevelTableScanner;
    
        sharpen::LevelTable *table_;
        std::unique_lock<sharpen::AsyncReadWriteLock> levelLock_;
        std::unique_lock<sharpen::AsyncReadWriteLock> memLock_;
        std::vector<sharpen::LevelViewItem> tables_;
        sharpen::Optional<std::pair<sharpen::ByteBuffer,sharpen::ByteBuffer>> range_;
        sharpen::ByteBuffer currentKey_;
        sharpen::Size currentTable_;
        bool useCache_;
    public:
    
        explicit LevelTableScanner(const sharpen::LevelTable &table);

        LevelTableScanner(const sharpen::LevelTable &table,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey);
    
        LevelTableScanner(Self &&other) noexcept = default;
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->table_ = other.table_;
                this->levelLock_ = std::move(other.levelLock_);
                this->memLock_ = std::move(other.memLock_);
                this->tables_ = std::move(other.tables_);
                this->range_ = std::move(other.range_);
                this->currentKey_ = std::move(other.currentKey_);
                this->currentTable_ = other.currentTable_;
                this->useCache_ = other.useCache_;
            }
            return *this;
        }
    
        ~LevelTableScanner() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
        inline void EnableCache() noexcept
        {
            this->useCache_ = true;
        }

        inline void DisableCache() noexcept
        {
            this->useCache_ = false;
        }

        inline bool IsEnableCache() const noexcept
        {
            return this->useCache_;
        }

        inline const sharpen::ByteBuffer &GetCurrentKey() const noexcept
        {
            return this->currentKey_;
        }

        inline bool IsRangeQuery() const noexcept
        {
            return this->range_.Exist();
        }

        inline bool IsFullTableQuery() const noexcept
        {
            return !this->range_.Exist();
        }

        inline const std::pair<sharpen::ByteBuffer,sharpen::ByteBuffer> &GetQueryRange() const noexcept
        {
            assert(this->IsRangeQuery());
            return this->range_.Get();
        }

        inline const sharpen::ByteBuffer &GetRangeBegin() const noexcept
        {
            return this->GetQueryRange().first;
        }

        inline const sharpen::ByteBuffer &GetRangeEnd() const noexcept
        {
            return this->GetQueryRange().second;
        }

        sharpen::ByteBuffer GetCurrentValue() const;

        bool Next();

        bool HasNext() const;

        void Seek(const sharpen::ByteBuffer &key);

        bool IsEmpty() const noexcept;
    };
}

#endif