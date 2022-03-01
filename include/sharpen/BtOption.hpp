#pragma once
#ifndef _SHARPEN_BTOPTION_HPP
#define _SHARPEN_BTOPTION_HPP

#include <utility>

#include "TypeDef.hpp"

namespace sharpen
{
    class BtOption
    {
    private:
        using Self = sharpen::BtOption;

        constexpr static sharpen::Size defaultCacheSize_{64};
        constexpr static sharpen::Uint16 defaultMaxRecordsOfBlock{128};
        
        sharpen::Uint16 maxRecordsOfBlock_;
        sharpen::Size cacheSize_;
    public:

        BtOption()
            :BtOption(Self::defaultMaxRecordsOfBlock)
        {}

        explicit BtOption(sharpen::Uint16 maxRecordOfBlock)
            :BtOption(maxRecordOfBlock,Self::defaultCacheSize_)
        {}

        BtOption(sharpen::Uint16 maxRecordOfBlock,sharpen::Size cacheSize)
            :maxRecordsOfBlock_(maxRecordOfBlock)
            ,cacheSize_(cacheSize)
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
    };
}

#endif