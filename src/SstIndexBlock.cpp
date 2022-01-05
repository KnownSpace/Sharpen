#include <sharpen/SstIndexBlock.hpp>

#include <stdexcept>

#include <sharpen/IteratorOps.hpp>

void sharpen::SstIndexBlock::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size size,sharpen::Size offset)
{
    if(buf.GetSize() - offset < size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    const char *begin = buf.Data() + offset;
    const char *end = begin + size;
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

sharpen::Size sharpen::SstIndexBlock::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    sharpen::Size size = buf.GetSize() - offset;
    sharpen::Size needSize{this->dataBlocks_.size()*(sizeof(sharpen::SstBlock) + sizeof(sharpen::Uint64))};
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        needSize += begin->Key().GetSize();
    }
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    char *data = buf.Data() + offset;
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        sharpen::Uint64 keySize = begin->Key().GetSize();
        std::memcpy(data,&keySize,sizeof(keySize));
        data += sizeof(keySize);
        if(keySize)
        {
            std::memcpy(data,begin->Key().Data(),keySize);
            data += keySize; 
        }
        std::memcpy(data,&begin->Block().offset_,sizeof(begin->Block().offset_));
        data += sizeof(begin->Block().offset_);
        std::memcpy(data,&begin->Block().size_,sizeof(begin->Block().size_));
        data += sizeof(begin->Block().size_);
    }
    return needSize;
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
    if (ite == this->End() || ite->Key() != key)
    {
        this->dataBlocks_.emplace_back(std::move(key),block);
    }
    else
    {
        ite->Key() = std::move(key);
        ite->Block() = block;
    }
    this->Sort();
}

void sharpen::SstIndexBlock::Put(sharpen::SstBlockHandle block)
{
    auto ite = this->Find(block.Key());
    if (ite == this->End() || ite->Key() != block.Key())
    {
        this->dataBlocks_.push_back(std::move(block));
    }
    else
    {
        *ite = std::move(block);
    }
    this->Sort();
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
    auto ite = this->Find(oldKey);
    if(ite != this->End() && ite->Key() == oldKey)
    {
        *ite = std::move(block);
        this->Sort();
    }
}

void sharpen::SstIndexBlock::Update(const sharpen::ByteBuffer &oldKey,sharpen::ByteBuffer newKey,const sharpen::SstBlock &block)
{
    auto ite = this->Find(oldKey);
    if(ite != this->End() && ite->Key() == oldKey)
    {
        ite->Key() = std::move(newKey);
        ite->Block() = block;
        this->Sort();
    }
}