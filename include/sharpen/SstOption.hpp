#pragma once
#ifndef _SHARPEN_SSTCACHEOPTION_HPP
#define _SHARPEN_SSTCACHEOPTION_HPP

#include <utility>

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    class SstOption
    {
    public:
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::SstOption;

        static constexpr std::size_t defaultDataCacheSize_{512};

        //1% - fake positive rate
        static constexpr std::size_t defaultFilterBitsOfElement_{10};

        std::size_t dataCacheSize_;
        std::size_t filterBitsOfElement_;
        std::size_t filtersCacheSize_;
        Comparator comp_;
    public:
    
        SstOption() noexcept
            :SstOption(Self::defaultFilterBitsOfElement_)
        {}

        explicit SstOption(std::size_t filterBitsOfElement) noexcept
            :SstOption(filterBitsOfElement,defaultDataCacheSize_)
        {}

        explicit SstOption(Comparator comp) noexcept
            :SstOption(comp,Self::defaultDataCacheSize_)
        {}

        SstOption(Comparator comp,std::size_t dataCacheSize) noexcept
            :SstOption(comp,Self::defaultFilterBitsOfElement_,dataCacheSize,dataCacheSize)
        {}

        SstOption(std::size_t filterBitsOfElement,std::size_t dataCacheSize) noexcept
            :SstOption(nullptr,filterBitsOfElement,dataCacheSize,dataCacheSize)
        {}

        SstOption(Comparator comp,std::size_t filterBitsOfElement,std::size_t dataCacheSize,std::size_t filtersCacheSize) noexcept
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

        inline std::size_t GetDataCacheSize() const noexcept
        {
            return this->dataCacheSize_;
        }

        inline std::size_t GetFilterCacheSize() const noexcept
        {
            if(!this->EnableFilter())
            {
                return 0;
            }
            return this->filtersCacheSize_;
        }

        inline std::size_t GetFilterBitsOfElement() const noexcept
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