#include <sharpen/LevelTableOption.hpp>

#include <cassert>

sharpen::LevelTableOption::LevelTableOption(std::size_t maxViewOfComponent,std::size_t maxTableOfComponent
                        ,std::size_t blockCacheSize,std::size_t filterBitsOfElement
                        ,std::size_t maxSizeOfMemoryTable,std::size_t maxCountOfImmutableTable
                        ,std::size_t blockSize,FileGenerator generator,Comparator comparator
                        ,std::size_t tableCacheSize)
    :maxViewOfComponent_(maxViewOfComponent)
    ,maxTableOfComponent_(maxTableOfComponent)
    ,blockCacheSize_(blockCacheSize)
    ,filterBitsOfElement_(filterBitsOfElement)
    ,maxSizeOfMem_(maxSizeOfMemoryTable)
    ,maxSizeOfImMems_(maxCountOfImmutableTable)
    ,blockSize_(blockSize)
    ,fileGenerator_(generator)
    ,comp_(comparator)
    ,tableCacheSize_(tableCacheSize)
{
    assert(this->maxTableOfComponent_ != 0 || this->maxViewOfComponent_ != 0);
    assert(this->blockSize_ != 0);
}

sharpen::LevelTableOption &sharpen::LevelTableOption::operator=(sharpen::LevelTableOption &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->maxViewOfComponent_ = other.maxViewOfComponent_;
        this->maxTableOfComponent_ = other.maxTableOfComponent_;
        this->blockCacheSize_ = other.blockCacheSize_;
        this->filterBitsOfElement_ = other.filterBitsOfElement_;
        this->maxSizeOfMem_ = other.maxSizeOfMem_;
        this->maxSizeOfImMems_ = other.maxSizeOfImMems_;
        this->blockSize_ = other.blockSize_;
        this->fileGenerator_ = other.fileGenerator_;
        this->comp_ = other.comp_;
    }
    return *this;
}