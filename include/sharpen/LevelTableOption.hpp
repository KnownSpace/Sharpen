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
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
        using FileGenerator = sharpen::FileChannelPtr(*)(const char*,sharpen::FileAccessModel,sharpen::FileOpenModel);
    
    public:
        static constexpr std::size_t defaultMaxViewOfComponent_{4};
        static constexpr std::size_t defaultMaxTableOfComponent_{16};
        //16MB / 4KB = 4096
        static constexpr std::size_t defaultBlockCacheSize_{4096};
        //1% - FPR
        static constexpr std::size_t defaultFilterBitsOfElement_{10};
        //8MB
        static constexpr std::size_t defaultMaxSizeOfMemoryTable_{1*1024*1024};
        static constexpr std::size_t defaultMaxCountOfImmutableTable_{8};
        //4KB
        static constexpr std::size_t defaultBlockSize_{4*1024};
        //16MB * 64 = 1GB
        static constexpr std::size_t defaultTableCacheSize_{64};

    private:

        std::size_t maxViewOfComponent_;
        std::size_t maxTableOfComponent_;
        std::size_t blockCacheSize_;
        std::size_t filterBitsOfElement_;
        std::size_t maxSizeOfMem_;
        std::size_t maxSizeOfImMems_;
        std::size_t blockSize_;
        FileGenerator fileGenerator_;
        Comparator comp_;
        std::size_t tableCacheSize_;
    public:

        LevelTableOption()
            :LevelTableOption(nullptr)
        {}

        explicit LevelTableOption(Comparator comp)
            :LevelTableOption(Self::defaultBlockCacheSize_,comp)
        {}

        LevelTableOption(std::size_t blockSize,Comparator comparator)
            :LevelTableOption(Self::defaultMaxViewOfComponent_,Self::defaultMaxTableOfComponent_,
                              Self::defaultBlockCacheSize_,Self::defaultFilterBitsOfElement_,
                              Self::defaultMaxSizeOfMemoryTable_,Self::defaultMaxCountOfImmutableTable_,
                              blockSize,nullptr,comparator,Self::defaultTableCacheSize_)
        {}
    
        LevelTableOption(std::size_t maxViewOfComponent,std::size_t maxTableOfComponent
                        ,std::size_t blockCacheSize,std::size_t filterBitsOfElement
                        ,std::size_t maxSizeOfMemoryTable,std::size_t maxCountOfImmutableTable
                        ,std::size_t blockSize,FileGenerator generator,Comparator comparator
                        ,std::size_t tableCacheSize);
    
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

        inline std::size_t GetMaxViewOfComponent() const noexcept
        {
            return this->maxViewOfComponent_;
        }

        inline std::size_t GetMaxTableOfComponent() const noexcept
        {
            return this->maxTableOfComponent_;
        }

        inline std::size_t GetBlockCacheSize() const noexcept
        {
            return this->blockCacheSize_;
        }

        inline std::size_t GetFilterBitsOfElement() const noexcept
        {
            return this->filterBitsOfElement_;
        }

        inline std::size_t GetMaxSizeOfMemoryTable() const noexcept
        {
            return this->maxSizeOfMem_;
        }

        inline std::size_t GetMaxCountOfImmutableTable() const noexcept
        {
            return this->maxSizeOfImMems_;
        }

        inline std::size_t GetBlockSize() const noexcept
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

        inline std::size_t GetTableCacheSize() const noexcept
        {
            return this->tableCacheSize_;
        }
    };
}

#endif