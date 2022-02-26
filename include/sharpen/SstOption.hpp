#pragma once
#ifndef _SHARPEN_SSTCACHEOPTION_HPP
#define _SHARPEN_SSTCACHEOPTION_HPP

#include <utility>

#include "TypeDef.hpp"

namespace sharpen
{
    class SstOption
    {
    private:
        using Self = sharpen::SstOption;
    
        static constexpr sharpen::Size defaultBlockSize_{4*1024};

        static constexpr sharpen::Size defaultDataCacheSize_{512};

        sharpen::Size blockSize_;
        sharpen::Size dataCacheSize_;
        sharpen::Size filtersCacheSize_;
    public:
    
        SstOption() noexcept
            :SstOption(defaultBlockSize_)
        {}

        explicit SstOption(sharpen::Size blockSize) noexcept
            :SstOption(blockSize,defaultDataCacheSize_)
        {}

        SstOption(sharpen::Size blockSize,sharpen::Size dataCacheSize) noexcept
            :SstOption(blockSize,dataCacheSize,dataCacheSize)
        {}

        SstOption(sharpen::Size blockSize,sharpen::Size dataCacheSize,sharpen::Size filtersCacheSize) noexcept
            :blockSize_(blockSize)
            ,dataCacheSize_(dataCacheSize)
            ,filtersCacheSize_(filtersCacheSize)
        {}
    
        SstOption(const Self &other) noexcept = default;
    
        SstOption(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other) noexcept
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->blockSize_ = other.blockSize_;
                this->dataCacheSize_ = other.dataCacheSize_;
                this->filtersCacheSize_ = other.filtersCacheSize_;
            }
            return *this;
        }
    
        ~SstOption() noexcept = default;

        inline sharpen::Size GetBlockSize() const
        {
            return this->blockSize_;
        }

        inline sharpen::Size GetDataCacheSize() const
        {
            return this->dataCacheSize_;
        }

        inline sharpen::Size GetFilterCacheSize() const
        {
            return this->filtersCacheSize_;
        }
    };   
}

#endif