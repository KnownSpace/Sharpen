#pragma once
#ifndef _SHARPEN_BTOPTION_HPP
#define _SHARPEN_BTOPTION_HPP

#include <utility>

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    class BtOption
    {
    public:
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::BtOption;

        constexpr static std::size_t defaultCacheSize_{64};
        constexpr static std::uint16_t defaultMaxRecordsOfBlock{128};
        
        std::uint16_t maxRecordsOfBlock_;
        std::size_t cacheSize_;
        Comparator comp_;
    public:

        BtOption() noexcept
            :BtOption(Self::defaultMaxRecordsOfBlock)
        {}

        explicit BtOption(std::uint16_t maxRecordOfBlock) noexcept
            :BtOption(nullptr,maxRecordOfBlock,Self::defaultCacheSize_)
        {}

        explicit BtOption(Comparator comp) noexcept
            :BtOption(comp,Self::defaultMaxRecordsOfBlock,Self::defaultCacheSize_)
        {}

        BtOption(Comparator comp,std::uint16_t maxRecordOfBlock,std::size_t cacheSize) noexcept
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

        inline std::size_t GetCacheSize() const noexcept
        {
            return this->cacheSize_;
        }

        inline std::uint16_t GetMaxRecordsOfBlock() const noexcept
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