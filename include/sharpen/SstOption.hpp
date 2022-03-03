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

        static constexpr sharpen::Size defaultDataCacheSize_{512};

        //1% - fake positive rate
        static constexpr sharpen::Size defaultFilterBitsOfElement_{10};

        sharpen::Size dataCacheSize_;
        sharpen::Size filterBitsOfElement_;
        sharpen::Size filtersCacheSize_;
    public:
    
        SstOption() noexcept
            :SstOption(Self::defaultFilterBitsOfElement_)
        {}

        explicit SstOption(sharpen::Size filterBitsOfElement) noexcept
            :SstOption(filterBitsOfElement,defaultDataCacheSize_)
        {}

        SstOption(sharpen::Size filterBitsOfElement,sharpen::Size dataCacheSize) noexcept
            :SstOption(filterBitsOfElement,dataCacheSize,dataCacheSize)
        {}

        SstOption(sharpen::Size filterBitsOfElement,sharpen::Size dataCacheSize,sharpen::Size filtersCacheSize) noexcept
            :dataCacheSize_(dataCacheSize)
            ,filterBitsOfElement_(filterBitsOfElement)
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
                this->dataCacheSize_ = other.dataCacheSize_;
                this->filterBitsOfElement_ = other.filterBitsOfElement_;
                this->filtersCacheSize_ = other.filtersCacheSize_;
            }
            return *this;
        }
    
        ~SstOption() noexcept = default;

        inline bool EnableFilter() const noexcept
        {
            return this->filterBitsOfElement_;
        }

        inline sharpen::Size GetDataCacheSize() const noexcept
        {
            return this->dataCacheSize_;
        }

        inline sharpen::Size GetFilterCacheSize() const noexcept
        {
            if(!this->EnableFilter())
            {
                return 0;
            }
            return this->filtersCacheSize_;
        }

        inline sharpen::Size GetFilterBitsOfElement() const noexcept
        {
            return this->filterBitsOfElement_;
        }
    };   
}

#endif