#pragma once
#ifndef _SHARPEN_BTOPTION_HPP
#define _SHARPEN_BTOPTION_HPP

#include <utility>

#include "TypeDef.hpp"

namespace sharpen
{
    class BtOption
    {
    public:
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::BtOption;

        constexpr static sharpen::Size defaultCacheSize_{64};
        constexpr static sharpen::Uint16 defaultMaxRecordsOfBlock{128};
        
        sharpen::Uint16 maxRecordsOfBlock_;
        sharpen::Size cacheSize_;
        Comparator comp_;
    public:

        BtOption() noexcept
            :BtOption(Self::defaultMaxRecordsOfBlock)
        {}

        explicit BtOption(sharpen::Uint16 maxRecordOfBlock) noexcept
            :BtOption(nullptr,maxRecordOfBlock,Self::defaultCacheSize_)
        {}

        explicit BtOption(Comparator comp) noexcept
            :BtOption(comp,Self::defaultMaxRecordsOfBlock,Self::defaultCacheSize_)
        {}

        BtOption(Comparator comp,sharpen::Uint16 maxRecordOfBlock,sharpen::Size cacheSize) noexcept
            :maxRecordsOfBlock_(maxRecordOfBlock)
            ,cacheSize_(cacheSize)
            ,comp_(comp)
        {}
    
        BtOption(const Self &other) = default;
    
        BtOption(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->cacheSize_ = other.cacheSize_;
                this->maxRecordsOfBlock_ = other.maxRecordsOfBlock_;
                this->comp_ = other.comp_;
            }
            return *this;
        }
    
        ~BtOption() noexcept = default;

        inline sharpen::Size GetCacheSize() const noexcept
        {
            return this->cacheSize_;
        }

        inline sharpen::Uint16 GetMaxRecordsOfBlock() const noexcept
        {
            return this->maxRecordsOfBlock_;
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }
    };
}

#endif