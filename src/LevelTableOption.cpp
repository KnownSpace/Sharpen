#include <sharpen/LevelTableOption.hpp>

#include <cassert>

sharpen::LevelTableOption::LevelTableOption(sharpen::Size maxViewOfComponent,sharpen::Size maxTableOfComponent
                        ,sharpen::Size blockCacheSize,sharpen::Size filterBitsOfElement
                        ,sharpen::Size maxSizeOfMemoryTable,sharpen::Size maxCountOfImmutableTable
                        ,sharpen::Size blockSize,FileGenerator generator,Comparator comparator
                        ,sharpen::Size tableCacheSize)
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