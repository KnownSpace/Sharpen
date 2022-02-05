#include <sharpen/PersistentTable.hpp>

#include <cassert>

sharpen::PersistentTable::PersistentTable(sharpen::FileChannelPtr channel)
    :PersistentTable(std::move(channel),Self::dataCacheSize_,Self::filterCacheSize_)
{}

sharpen::PersistentTable::PersistentTable(sharpen::FileChannelPtr channel,sharpen::Size dataCache,sharpen::Size filterCache)
    :PersistentTable(std::move(channel),10,dataCache,filterCache)
{}

sharpen::PersistentTable::PersistentTable(sharpen::FileChannelPtr channel,sharpen::Size bitsOfElements,sharpen::Size dataCache,sharpen::Size filterCache)
    :channel_(std::move(channel))
    ,root_()
    ,filterBits_(bitsOfElements)
    ,dataCache_(dataCache)
    ,filterCache_(bitsOfElements != 0 ? filterCache:0)
{
    this->LoadRoot();
}

void sharpen::PersistentTable::LoadRoot()
{
    sharpen::Uint64 size{this->channel_->GetFileSize()};
    if(size)
    {
        this->root_.LoadFrom(this->channel_);
    }
}

bool sharpen::PersistentTable::Empty() const
{
    return this->root_.IndexBlock().Empty();
}

sharpen::SstDataBlock sharpen::PersistentTable::LoadDataBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    return sharpen::SortedStringTableBuilder::LoadDataBlock<sharpen::SstDataBlock>(this->channel_,offset,size);
}

sharpen::SstDataBlock sharpen::PersistentTable::LoadDataBlock(const sharpen::ByteBuffer &key) const
{
    auto ite = this->root_.IndexBlock().Find(key);
    if(ite == this->root_.IndexBlock().End())
    {
        throw std::invalid_argument("key doesn't exist");
    }
    return this->LoadDataBlock(ite->Block().offset_,ite->Block().size_);
}

std::shared_ptr<sharpen::SstDataBlock> sharpen::PersistentTable::LoadDataBlockCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    std::shared_ptr<sharpen::SstDataBlock> block{this->dataCache_.Get(cacheKey)};
    if(!block)
    {
        block = std::make_shared<sharpen::SstDataBlock>(this->LoadDataBlock(offset,size));
        block = this->dataCache_.GetOrEmplace(cacheKey,std::move(*block));
    }
    return block;
}

sharpen::BloomFilter<sharpen::ByteBuffer> sharpen::PersistentTable::LoadFilter(const sharpen::ByteBuffer &key) const
{
    auto ite = this->root_.MetaIndexBlock().Find(key);
    if(ite == this->root_.MetaIndexBlock().End())
    {
        throw std::invalid_argument("key doen't exist");
    }
    assert(this->filterBits_);
    return sharpen::SortedStringTableBuilder::LoadFilter(this->channel_,ite->Block().offset_,ite->Block().size_,this->filterBits_);
}

std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> sharpen::PersistentTable::LoadFilterCache(const std::string &cacheKey,sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{this->filterCache_.Get(cacheKey)};
    if(!filter)
    {
        sharpen::ByteBuffer buf{sharpen::IntCast<sharpen::Size>(size)};
        this->channel_->ReadAsync(buf,offset);
        filter = this->filterCache_.GetOrEmplace(cacheKey,buf.Data(),buf.GetSize(),static_cast<sharpen::Size>(10));
    }
    return filter;
}

sharpen::ExistStatus sharpen::PersistentTable::Exist(const sharpen::ByteBuffer &key) const
{
    auto filterIte = this->root_.MetaIndexBlock().Find(key);
    if(filterIte == this->root_.MetaIndexBlock().End())
    {
        return sharpen::ExistStatus::NotExist;
    }
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{nullptr};
    std::string cacheKey{filterIte->GetKey().Data(),filterIte->GetKey().GetSize()};
    filter = this->LoadFilterCache(cacheKey,filterIte->Block().offset_,filterIte->Block().size_);
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
    cacheKey.assign(blockIte->GetKey().Data(),blockIte->GetKey().GetSize());
    block = this->LoadDataBlockCache(cacheKey,blockIte->Block().offset_,blockIte->Block().size_);
    return block->Exist(key);
}

std::shared_ptr<const sharpen::SstDataBlock> sharpen::PersistentTable::QueryDataBlock(const sharpen::ByteBuffer &key) const
{
    auto filterIte = this->root_.MetaIndexBlock().Find(key);
    if(filterIte == this->root_.MetaIndexBlock().End())
    {
        return nullptr;
    }
    std::shared_ptr<sharpen::BloomFilter<sharpen::ByteBuffer>> filter{nullptr};
    std::string cacheKey{filterIte->GetKey().Data(),filterIte->GetKey().GetSize()};
    filter = this->LoadFilterCache(cacheKey,filterIte->Block().offset_,filterIte->Block().size_);
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
    cacheKey.assign(blockIte->GetKey().Data(),blockIte->GetKey().GetSize());
    block = this->LoadDataBlockCache(cacheKey,blockIte->Block().offset_,blockIte->Block().size_);
    return block;
}

sharpen::ByteBuffer sharpen::PersistentTable::Query(const sharpen::ByteBuffer &key) const
{
    auto block = this->QueryDataBlock(key);
    if(!block)
    {
        throw std::invalid_argument("key doesn't exist");
    }
    return block->Get(key);
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::PersistentTable::TryQuery(const sharpen::ByteBuffer &key) const
{
    auto block = this->QueryDataBlock(key);
    if(!block || block->Exist(key) == sharpen::ExistStatus::NotExist)
    {
        return sharpen::EmptyOpt;
    }
    return block->Get(key);
}

sharpen::PersistentTable &sharpen::PersistentTable::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->channel_ = std::move(other.channel_);
        this->root_ = std::move(other.root_);
        this->filterBits_ = other.filterBits_;
        this->dataCache_ = std::move(other.dataCache_);
        this->filterCache_ = std::move(other.filterCache_);
    }
    return *this;
}