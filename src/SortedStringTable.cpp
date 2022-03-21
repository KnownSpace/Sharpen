#include <sharpen/SortedStringTable.hpp>

#include <cassert>

sharpen::SortedStringTable::SortedStringTable(sharpen::FileChannelPtr channel)
    :SortedStringTable(std::move(channel),sharpen::SstOption{})
{}

sharpen::SortedStringTable::SortedStringTable(sharpen::FileChannelPtr channel,const sharpen::SstOption &opt)
    :channel_(std::move(channel))
    ,root_()
    ,filterBitsOfElement_(opt.GetFilterBitsOfElement())
    ,dataCache_(opt.GetDataCacheSize())
    ,filterCache_(opt.GetFilterCacheSize())
    ,comp_(opt.GetComparator())
{
    this->root_.IndexBlock().SetComparator(this->comp_);
    this->root_.MetaIndexBlock().SetComparator(this->comp_);
    this->LoadRoot();
}

void sharpen::SortedStringTable::LoadRoot()
{
    sharpen::Uint64 size{this->channel_->GetFileSize()};
    if(size)
    {
        this->root_.LoadFrom(this->channel_);
    }
}

bool sharpen::SortedStringTable::Empty() const
{
    return this->root_.IndexBlock().Empty();
}

sharpen::Int32 sharpen::SortedStringTable::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::SstDataBlock sharpen::SortedStringTable::LoadBlock(sharpen::FilePointer pointer) const
{
    return sharpen::SortedStringTableBuilder::LoadDataBlock<sharpen::SstDataBlock>(this->channel_,pointer.offset_,pointer.size_,this->comp_);
}

sharpen::SstDataBlock sharpen::SortedStringTable::LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    return sharpen::SortedStringTableBuilder::LoadDataBlock<sharpen::SstDataBlock>(this->channel_,offset,size,this->comp_);
}

sharpen::SstDataBlock sharpen::SortedStringTable::LoadBlock(const sharpen::ByteBuffer &key) const
{
    auto ite = this->root_.IndexBlock().Find(key);
    if(ite == this->root_.IndexBlock().End())
    {
        throw std::out_of_range("key doesn't exist(block not found)");
    }
    return this->LoadBlock(ite->Block().offset_,ite->Block().size_);
}

std::shared_ptr<sharpen::SstDataBlock> sharpen::SortedStringTable::LoadBlockCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    std::shared_ptr<sharpen::SstDataBlock> block{this->dataCache_.Get(cacheKey.Begin(),cacheKey.End())};
    if(!block)
    {
        block = std::make_shared<sharpen::SstDataBlock>(this->LoadBlock(offset,size));
        block = this->dataCache_.GetOrEmplace(cacheKey.Begin(),cacheKey.End(),std::move(*block));
    }
    return block;
}

sharpen::BloomFilter<sharpen::ByteBuffer> sharpen::SortedStringTable::LoadFilter(const sharpen::ByteBuffer &key) const
{
    auto ite = this->root_.MetaIndexBlock().Find(key);
    if(ite == this->root_.MetaIndexBlock().End())
    {
        throw std::out_of_range("key doen't exist");
    }
    return sharpen::SortedStringTableBuilder::LoadFilter(this->channel_,ite->Block().offset_,ite->Block().size_,this->filterBitsOfElement_);
}

std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> sharpen::SortedStringTable::LoadFilterCache(const sharpen::ByteBuffer &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{this->filterCache_.Get(cacheKey.Begin(),cacheKey.End())};
    if(!filter)
    {
        sharpen::ByteBuffer buf{sharpen::IntCast<sharpen::Size>(size)};
        this->channel_->ReadAsync(buf,offset);
        filter = this->filterCache_.GetOrEmplace(cacheKey.Begin(),cacheKey.End(),buf.Data(),buf.GetSize(),static_cast<sharpen::Size>(10));
    }
    return filter;
}

sharpen::ExistStatus sharpen::SortedStringTable::Exist(const sharpen::ByteBuffer &key) const
{
    auto filterIte = this->root_.MetaIndexBlock().Find(key);
    if(filterIte == this->root_.MetaIndexBlock().End())
    {
        return sharpen::ExistStatus::NotExist;
    }
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{nullptr};
    const sharpen::ByteBuffer *cacheKey = &filterIte->GetKey();
    filter = this->LoadFilterCache(*cacheKey,filterIte->Block().offset_,filterIte->Block().size_);
    if(!filter->Containe(key))
    {
        return sharpen::ExistStatus::NotExist;
    }
    auto blockIte = this->root_.IndexBlock().Find(key);
    if(blockIte == this->root_.IndexBlock().End())
    {
        return sharpen::ExistStatus::NotExist;
    }
    std::shared_ptr<sharpen::SstDataBlock> block{nullptr};
    cacheKey = &blockIte->GetKey();
    block = this->LoadBlockCache(*cacheKey,blockIte->Block().offset_,blockIte->Block().size_);
    return block->Exist(key);
}

std::shared_ptr<const sharpen::SstDataBlock> sharpen::SortedStringTable::FindBlockFromCache(const sharpen::ByteBuffer &key) const
{
    auto blockIte = this->root_.IndexBlock().Find(key);
    if(blockIte == this->root_.IndexBlock().End())
    {
        return nullptr;
    }
    std::shared_ptr<sharpen::SstDataBlock> block{this->dataCache_.Get(key.Begin(),key.End())};
    return block;
}

std::shared_ptr<const sharpen::SstDataBlock> sharpen::SortedStringTable::FindBlock(const sharpen::ByteBuffer &key,bool doCache) const
{
    auto filterIte = this->root_.MetaIndexBlock().Find(key);
    if(filterIte == this->root_.MetaIndexBlock().End())
    {
        return nullptr;
    }
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{nullptr};
    const sharpen::ByteBuffer *cacheKey = &filterIte->GetKey();
    filter = this->LoadFilterCache(*cacheKey,filterIte->Block().offset_,filterIte->Block().size_);
    if(!filter->Containe(key))
    {
        return nullptr;
    }
    auto blockIte = this->root_.IndexBlock().Find(key);
    if(blockIte == this->root_.IndexBlock().End())
    {
        return nullptr;
    }
    std::shared_ptr<sharpen::SstDataBlock> block{nullptr};
    cacheKey = &blockIte->GetKey();
    if(doCache)
    {
        block = this->LoadBlockCache(*cacheKey,blockIte->Block().offset_,blockIte->Block().size_);
    }
    else
    {
        block = this->dataCache_.Get(cacheKey->Begin(),cacheKey->End());
        if(!block)
        {
            block = std::make_shared<sharpen::SstDataBlock>(this->LoadBlock(blockIte->Block().offset_,blockIte->Block().size_));
        }
    }
    return block;
}

sharpen::ByteBuffer sharpen::SortedStringTable::Get(const sharpen::ByteBuffer &key) const
{
    std::shared_ptr<const sharpen::SstDataBlock> block = this->FindBlock(key);
    if(!block)
    {
        throw std::out_of_range("key doesn't exists(block not found)");
    }
    return block->Get(key);
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::SortedStringTable::TryGet(const sharpen::ByteBuffer &key) const
{
    std::shared_ptr<const sharpen::SstDataBlock> block = this->FindBlock(key);
    if(!block || block->Exist(key) == sharpen::ExistStatus::NotExist)
    {
        return sharpen::EmptyOpt;
    }
    return block->Get(key);
}

sharpen::SortedStringTable &sharpen::SortedStringTable::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->channel_ = std::move(other.channel_);
        this->root_ = std::move(other.root_);
        this->dataCache_ = std::move(other.dataCache_);
        this->filterCache_ = std::move(other.filterCache_);
    }
    return *this;
}