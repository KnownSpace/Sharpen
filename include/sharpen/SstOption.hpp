#pragma once
#ifndef _SHARPEN_SSTCACHEOPTION_HPP
#define _SHARPEN_SSTCACHEOPTION_HPP

#include <utility>

#include "TypeDef.hpp"

namespace sharpen
{
    class SstOption
    {
    public:
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::SstOption;

        static constexpr sharpen::Size defaultDataCacheSize_{512};

        //1% - fake positive rate
        static constexpr sharpen::Size defaultFilterBitsOfElement_{10};

        sharpen::Size dataCacheSize_;
        sharpen::Size filterBitsOfElement_;
        sharpen::Size filtersCacheSize_;
        Comparator comp_;
    public:
    
        SstOption() noexcept
            :SstOption(Self::defaultFilterBitsOfElement_)
        {}

        explicit SstOption(sharpen::Size filterBitsOfElement) noexcept
            :SstOption(filterBitsOfElement,defaultDataCacheSize_)
        {}

        explicit SstOption(Comparator comp) noexcept
            :SstOption(comp,Self::defaultDataCacheSize_)
        {}

        SstOption(Comparator comp,sharpen::Size dataCacheSize) noexcept
            :SstOption(comp,Self::defaultFilterBitsOfElement_,dataCacheSize,dataCacheSize)
        {}

        SstOption(sharpen::Size filterBitsOfElement,sharpen::Size dataCacheSize) noexcept
            :SstOption(nullptr,filterBitsOfElement,dataCacheSize,dataCacheSize)
        {}

        SstOption(Comparator comp,sharpen::Size filterBitsOfElement,sharpen::Size dataCacheSize,sharpen::Size filtersCacheSize) noexcept
            :dataCacheSize_(dataCacheSize)
            ,filterBitsOfElement_(filterBitsOfElement)
            ,filtersCacheSize_(filtersCacheSize)
            ,comp_(comp)
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
                this->comp_ = other.comp_;
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

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }
    };   
}

#endif