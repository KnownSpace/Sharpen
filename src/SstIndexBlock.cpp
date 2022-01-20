#include <sharpen/SstIndexBlock.hpp>

#include <stdexcept>
#include <cassert>

#include <sharpen/IteratorOps.hpp>

void sharpen::SstIndexBlock::LoadFrom(const char *data,sharpen::Size size)
{
    const char *begin = data;
    const char *end = begin + size;
    this->dataBlocks_.clear();
    while (begin != end)
    {
        if(sharpen::GetRangeSize(begin,end) < sizeof(sharpen::SstBlock) + sizeof(sharpen::Uint64))
        {
            this->dataBlocks_.clear();
            throw std::invalid_argument("invalid buffer");
        }
        sharpen::Uint64 keySize{0};
        std::memcpy(&keySize,begin,sizeof(keySize));
        begin += 8;
        sharpen::ByteBuffer key{begin,keySize};
        begin += keySize;
        sharpen::SstBlock block;
        if(sharpen::GetRangeSize(begin,end) < sizeof(block))
        {
            this->dataBlocks_.clear();
            throw std::invalid_argument("invalid buffer");
        }
        std::memcpy(&block.offset_,begin,sizeof(block.offset_));
        begin += sizeof(block.offset_);
        std::memcpy(&block.size_,begin,sizeof(block.size_));
        begin += sizeof(block.size_);
        this->dataBlocks_.emplace_back(std::move(key),block);
    }
}

void sharpen::SstIndexBlock::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size size,sharpen::Size offset)
{
    assert(buf.GetSize() > offset);
    if(buf.GetSize() - offset < size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    this->LoadFrom(buf.Data() + offset,size);
}

sharpen::Size sharpen::SstIndexBlock::ComputeNeedSize() const noexcept
{
    sharpen::Size needSize{this->dataBlocks_.size()*(sizeof(sharpen::SstBlock) + sizeof(sharpen::Uint64))};
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        needSize += begin->Key().GetSize();
    }
    return needSize;
}

sharpen::Size sharpen::SstIndexBlock::UnsafeStoreTo(char *data) const noexcept
{
    sharpen::Size offset{0};
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        sharpen::Uint64 keySize = begin->Key().GetSize();
        std::memcpy(data + offset,&keySize,sizeof(keySize));
        offset += sizeof(keySize);
        if(keySize)
        {
            std::memcpy(data + offset,begin->Key().Data(),keySize);
            offset += keySize; 
        }
        std::memcpy(data + offset,&begin->Block().offset_,sizeof(begin->Block().offset_));
        offset += sizeof(begin->Block().offset_);
        std::memcpy(data + offset,&begin->Block().size_,sizeof(begin->Block().size_));
        offset += sizeof(begin->Block().size_);
    }
    return offset;
}

sharpen::Size sharpen::SstIndexBlock::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeNeedSize()};
    if(needSize > size)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::SstIndexBlock::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() > offset);
    sharpen::Size size{buf.GetSize() - offset};
    sharpen::Size needSize{this->ComputeNeedSize()};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    char *data = buf.Data() + offset;
    return this->UnsafeStoreTo(data);
}

sharpen::SstIndexBlock::ConstIterator sharpen::SstIndexBlock::Find(const sharpen::ByteBuffer &key) const noexcept
{
    auto begin = this->dataBlocks_.cbegin();
    auto end = this->dataBlocks_.cend();
    while (begin != end)
    {
        sharpen::Size size = sharpen::GetRangeSize(begin,end);
        auto mid = begin + size/2;
        if(mid->Key() == key)
        {
            return mid;
        }
        else if(mid->Key() > key)
        {
            end = mid;
        }
        else if(mid->Key() < key)
        {
            begin = sharpen::IteratorForward(mid,1);
        }
    }
    return begin;
}

sharpen::SstIndexBlock::Iterator sharpen::SstIndexBlock::Find(const sharpen::ByteBuffer &key) noexcept
{
    auto begin = this->dataBlocks_.begin();
    auto end = this->dataBlocks_.end();
    while (begin != end)
    {
        sharpen::Size size = sharpen::GetRangeSize(begin,end);
        auto mid = begin + size/2;
        if(mid->Key() == key)
        {
            return mid;
        }
        else if(mid->Key() > key)
        {
            end = mid;
        }
        else if(mid->Key() < key)
        {
            begin = sharpen::IteratorForward(mid,1);
        }
    }
    return begin;
}

void sharpen::SstIndexBlock::Put(sharpen::ByteBuffer key,const sharpen::SstBlock &block)
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        if(ite->Key() == key)
        {
            ite->Block() = block;
            return;
        }
        this->dataBlocks_.emplace(ite,std::move(key),block);
        return;
    }
    this->dataBlocks_.emplace_back(std::move(key),block);
}

void sharpen::SstIndexBlock::Put(sharpen::SstBlockHandle block)
{
    auto ite = this->Find(block.Key());
    if(ite != this->End())
    {
        if(ite->Key() == block.Key())
        {
            ite->Block() = std::move(block.Block());
            return;
        }
        this->dataBlocks_.emplace(ite,std::move(block));
        return;
    }
    this->dataBlocks_.emplace_back(std::move(block));
}

void sharpen::SstIndexBlock::Delete(const sharpen::ByteBuffer &key) noexcept
{
    auto ite = this->Find(key);
    if (ite != this->End() && ite->Key() == key)
    {
        this->dataBlocks_.erase(ite);
    }
}

void sharpen::SstIndexBlock::Update(const sharpen::ByteBuffer &oldKey,sharpen::SstBlockHandle block)
{
    this->Delete(oldKey);
    this->Put(std::move(block));
}

void sharpen::SstIndexBlock::Update(const sharpen::ByteBuffer &oldKey,sharpen::ByteBuffer newKey,const sharpen::SstBlock &block)
{
    this->Delete(oldKey);
    this->Put(std::move(newKey),std::move(block));
}