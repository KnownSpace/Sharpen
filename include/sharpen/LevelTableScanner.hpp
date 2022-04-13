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
        sharpen::Optional<std::pair<sharpen::ByteBuffer,sharpen::ByteBuffer>> range_;
        sharpen::ByteBuffer currentKey_;
        sharpen::Size currentPointer_;
        bool useCache_;
    public:
    
        LevelTableScanner();
    
        LevelTableScanner(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~LevelTableScanner() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif