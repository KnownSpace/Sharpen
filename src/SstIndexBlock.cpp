#include <sharpen/SstIndexBlock.hpp>

#include <stdexcept>
#include <cassert>

#include <sharpen/IteratorOps.hpp>

sharpen::SstIndexBlock::SstIndexBlock()
    :dataBlocks_()
    ,comp_(nullptr)
{}

void sharpen::SstIndexBlock::LoadFrom(const char *data,std::size_t size)
{
    const char *begin = data;
    const char *end = begin + size;
    this->dataBlocks_.clear();
    while (begin != end)
    {
        if(sharpen::GetRangeSize(begin,end) < sizeof(sharpen::FilePointer) + sizeof(std::uint64_t))
        {
            this->dataBlocks_.clear();
            throw std::invalid_argument("invalid buffer");
        }
        std::uint64_t keySize{0};
        std::memcpy(&keySize,begin,sizeof(keySize));
        begin += 8;
        sharpen::ByteBuffer key{begin,keySize};
        begin += keySize;
        sharpen::FilePointer block;
        if(sharpen::GetRangeSize(begin,end) < sizeof(block))
        {
            this->dataBlocks_.clear();
            throw std::invalid_argument("invalid buffer");
        }
        std::memcpy(&block,begin,sizeof(block));
        begin += sizeof(block);
        this->dataBlocks_.emplace_back(std::move(key),block);
    }
}

void sharpen::SstIndexBlock::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::SstIndexBlock::ComputeNeedSize() const noexcept
{
    std::size_t needSize{this->dataBlocks_.size()*(sizeof(sharpen::FilePointer) + sizeof(std::uint64_t))};
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        needSize += begin->GetKey().GetSize();
    }
    return needSize;
}

std::size_t sharpen::SstIndexBlock::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    for (auto begin = this->dataBlocks_.begin(),end = this->dataBlocks_.end(); begin != end; ++begin)
    {
        std::uint64_t keySize = begin->GetKey().GetSize();
        std::memcpy(data + offset,&keySize,sizeof(keySize));
        offset += sizeof(keySize);
        if(keySize)
        {
            std::memcpy(data + offset,begin->GetKey().Data(),keySize);
            offset += keySize; 
        }
        std::memcpy(data + offset,&begin->Block(),sizeof(begin->Block()));
        offset += sizeof(begin->Block());
    }
    return offset;
}

std::size_t sharpen::SstIndexBlock::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeNeedSize()};
    if(needSize > size)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::SstIndexBlock::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    std::size_t needSize{this->ComputeNeedSize()};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

bool sharpen::SstIndexBlock::WarppedComp(Comparator comp,const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept
{
    if(comp)
    {
        return comp(block.GetKey(),key) == -1;
    }
    return Self::Comp(block,key);
}

bool sharpen::SstIndexBlock::Comp(const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept
{
    return block.GetKey() < key;
}

std::int32_t sharpen::SstIndexBlock::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::SstIndexBlock::ConstIterator sharpen::SstIndexBlock::Find(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstBlockHandle&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->dataBlocks_.begin(),this->dataBlocks_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::SstBlockHandle&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->dataBlocks_.begin(),this->dataBlocks_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::SstIndexBlock::Iterator sharpen::SstIndexBlock::Find(const sharpen::ByteBuffer &key) noexcept
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstBlockHandle&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->dataBlocks_.begin(),this->dataBlocks_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::SstBlockHandle&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->dataBlocks_.begin(),this->dataBlocks_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

void sharpen::SstIndexBlock::Put(sharpen::ByteBuffer key,const sharpen::FilePointer &block)
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        if(this->CompKey(ite->GetKey(),key) == 0)
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
    auto ite = this->Find(block.GetKey());
    if(ite != this->End())
    {
        if(this->CompKey(ite->GetKey(),block.GetKey()) == 0)
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
    if (ite != this->End() && this->CompKey(ite->GetKey(),key) == 0)
    {
        this->dataBlocks_.erase(ite);
    }
}