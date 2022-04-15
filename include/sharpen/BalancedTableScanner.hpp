#pragma once
#ifndef _SHARPEN_BALANCEDTABLESCANNER_HPP
#define _SHARPEN_BALANCEDTABLESCANNER_HPP

#include "AsyncReadWriteLock.hpp"
#include "BtBlock.hpp"
#include "ByteBuffer.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class BalancedTable;

    class BalancedTableScanner:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::BalancedTableScanner;

        bool HasNext(const sharpen::BtBlock &block,const sharpen::ByteBuffer &key) const;
    
        const sharpen::BalancedTable *table_;
        std::unique_lock<sharpen::AsyncReadWriteLock> rootLock_;
        std::vector<sharpen::FilePointer> pointers_;
        sharpen::Optional<std::pair<sharpen::ByteBuffer,sharpen::ByteBuffer>> range_;
        sharpen::ByteBuffer currentKey_;
        sharpen::Size currentPointer_;
        bool useCache_;
    public:
    
        explicit BalancedTableScanner(const sharpen::BalancedTable &table);

        BalancedTableScanner(const sharpen::BalancedTable &table,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey);
    
        BalancedTableScanner(Self &&other) noexcept = default;
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->table_ = other.table_;
                this->rootLock_ = std::move(other.rootLock_);
                this->pointers_ = std::move(other.pointers_);
                this->range_ = std::move(other.range_);
                this->currentKey_ = std::move(other.currentKey_);
                this->currentPointer_ = other.currentPointer_;
                this->useCache_ = other.useCache_;
            }
            return *this;
        }
    
        ~BalancedTableScanner() noexcept = default;
    
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

        bool Seek(const sharpen::ByteBuffer &key);

        inline bool Empty() const noexcept
        {
            return this->pointers_.empty();
        }
    };
}

#endif