#include <sharpen/SstIndexBlock.hpp>

#include <stdexcept>

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
        if(end - begin < sizeof(sharpen::SstBlock) + sizeof(sharpen::Uint64))
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
        if(end - begin < sizeof(block))
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