#pragma once
#ifndef _SHARPEN_PERSISTENTTABLE_HPP
#define _SHARPEN_PERSISTENTTABLE_HPP

#include "SortedStringTable.hpp"
#include "SstDataBlock.hpp"
#include "SegmentedCircleCache.hpp"
#include "BloomFilter.hpp"

namespace sharpen
{
    class PersistentTable
    {
    private:
        using Self = sharpen::PersistentTable;
    
        sharpen::SortedStringTable root_;
        sharpen::SegmentedCircleCache<sharpen::SstDataBlock> cache_;
    public:
    
        PersistentTable();
    
        PersistentTable(const Self &other);
    
        PersistentTable(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~PersistentTable() noexcept = default;
    };
}

#endif