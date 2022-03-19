#pragma once
#ifndef _SHARPEN_LEVELTABLEOPTION_HPP
#define _SHARPEN_LEVELTABLEOPTION_HPP

#include <utility>

#include "IFileChannel.hpp"

namespace sharpen
{
    class LevelTableOption
    {
    private:
        using Self = sharpen::LevelTableOption;
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
        using FileGenerator = sharpen::FileChannelPtr(*)(const char*,sharpen::FileAccessModel,sharpen::FileOpenModel);
    
    public:
        static constexpr sharpen::Size defaultMaxViewOfComponent_{4};
        static constexpr sharpen::Size defaultMaxTableOfComponent_{16};
        //16MB / 4KB = 4096
        static constexpr sharpen::Size defaultBlockCacheSize_{4096};
        //1% - FPR
        static constexpr sharpen::Size defaultFilterBitsOfElement_{10};
        //16MB
        static constexpr sharpen::Size defaultMaxSizeOfMemoryTable_{1*1024*1024};
        static constexpr sharpen::Size defaultMaxCountOfImmutableTable_{16};
        //4KB
        static constexpr sharpen::Size defaultBlockSize_{4*1024};
        //16MB * 64 = 1GB
        static constexpr sharpen::Size defaultTableCacheSize_{64};

    private:

        sharpen::Size maxViewOfComponent_;
        sharpen::Size maxTableOfComponent_;
        sharpen::Size blockCacheSize_;
        sharpen::Size filterBitsOfElement_;
        sharpen::Size maxSizeOfMem_;
        sharpen::Size maxSizeOfImMems_;
        sharpen::Size blockSize_;
        FileGenerator fileGenerator_;
        Comparator comp_;
        sharpen::Size tableCacheSize_;
    public:

        LevelTableOption()
            :LevelTableOption(nullptr)
        {}

        explicit LevelTableOption(Comparator comp)
            :LevelTableOption(Self::defaultBlockCacheSize_,comp)
        {}

        LevelTableOption(sharpen::Size blockSize,Comparator comparator)
            :LevelTableOption(Self::defaultMaxViewOfComponent_,Self::defaultMaxTableOfComponent_,
                              Self::defaultBlockCacheSize_,Self::defaultFilterBitsOfElement_,
                              Self::defaultMaxSizeOfMemoryTable_,Self::defaultMaxCountOfImmutableTable_,
                              blockSize,nullptr,comparator,Self::defaultTableCacheSize_)
        {}
    
        LevelTableOption(sharpen::Size maxViewOfComponent,sharpen::Size maxTableOfComponent
                        ,sharpen::Size blockCacheSize,sharpen::Size filterBitsOfElement
                        ,sharpen::Size maxSizeOfMemoryTable,sharpen::Size maxCountOfImmutableTable
                        ,sharpen::Size blockSize,FileGenerator generator,Comparator comparator
                        ,sharpen::Size tableCacheSize);
    
        LevelTableOption(const Self &other) = default;
    
        LevelTableOption(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~LevelTableOption() noexcept = default;

        inline sharpen::Size GetMaxViewOfComponent() const noexcept
        {
            return this->maxViewOfComponent_;
        }

        inline sharpen::Size GetMaxTableOfComponent() const noexcept
        {
            return this->maxTableOfComponent_;
        }

        inline sharpen::Size GetBlockCacheSize() const noexcept
        {
            return this->blockCacheSize_;
        }

        inline sharpen::Size GetFilterBitsOfElement() const noexcept
        {
            return this->filterBitsOfElement_;
        }

        inline sharpen::Size GetMaxSizeOfMemoryTable() const noexcept
        {
            return this->maxSizeOfMem_;
        }

        inline sharpen::Size GetMaxCountOfImmutableTable() const noexcept
        {
            return this->maxSizeOfImMems_;
        }

        inline sharpen::Size GetBlockSize() const noexcept
        {
            return this->blockSize_;
        }

        inline FileGenerator GetFileGenerator() const noexcept
        {
            return this->fileGenerator_;
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }

        inline sharpen::Size GetTableCacheSize() const noexcept
        {
            return this->tableCacheSize_;
        }
    };
}

#endif