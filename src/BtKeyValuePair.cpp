#include <sharpen/BtKeyValuePair.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::BtKeyValuePair::BtKeyValuePair(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
    :key_(std::move(key))
    ,value_(std::move(value))
{}

std::size_t sharpen::BtKeyValuePair::ComputeSize(const sharpen::ByteBuffer &key,const sharpen::ByteBuffer &value) noexcept
{
    std::size_t size{0};
    //key size
    sharpen::Varuint64 builder{key.GetSize()};
    size += builder.ComputeSize();
    //value size
    builder.Set(value.GetSize());
    size += builder.ComputeSize();
    //key
    size += key.GetSize();
    //value
    size += value.GetSize();
    return size;
}

std::size_t sharpen::BtKeyValuePair::ComputeSize() const noexcept
{
    return Self::ComputeSize(this->key_,this->value_);
}

std::size_t sharpen::BtKeyValuePair::UnsafeStoreTo(char *data) const
{
    std::size_t offset{0};
    //store key size
    sharpen::Varuint64 builder{this->key_.GetSize()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),size);
    offset += size;
    //store value size
    builder.Set(this->value_.GetSize());
    size = builder.ComputeSize();
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //store key
    size = this->key_.GetSize();
    std::memcpy(data + offset,this->key_.Data(),size);
    offset += size;
    //store value
    size = this->value_.GetSize();
    std::memcpy(data + offset,this->value_.Data(),size);
    offset += size;
    return offset;
}

std::size_t sharpen::BtKeyValuePair::LoadFrom(const char *data,std::size_t size)
{
    if(size < 2)
    {
        throw std::invalid_argument("invalid buffer");
    }
    //load key size
    std::size_t keySize{0};
    std::size_t valueSize{0};
    std::size_t offset{0};
    sharpen::Varuint64 builder{data,size};
    offset += builder.ComputeSize();
    if (offset == size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    keySize = sharpen::IntCast<std::size_t>(builder.Get());
    //load value size
    builder.Set(data + offset,size - offset);
    offset += builder.ComputeSize();
    if (offset == size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    valueSize = sharpen::IntCast<std::size_t>(builder.Get());
    //load key
    sharpen::ByteBuffer buf{keySize};
    if(size < offset + keySize)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::memcpy(buf.Data(),data + offset,keySize);
    offset += keySize;
    this->key_ = std::move(buf);
    //load value
    if(valueSize)
    {
        buf.ExtendTo(valueSize);
        if(size < offset + valueSize)
        {
            throw std::invalid_argument("invalid buffer");
        }
        std::memcpy(buf.Data(),data + offset,valueSize);
        offset += valueSize;
        this->value_ = std::move(buf);
    }
    return offset;
}

std::size_t sharpen::BtKeyValuePair::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::BtKeyValuePair::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    this->UnsafeStoreTo(data);
    return needSize;
}

std::size_t sharpen::BtKeyValuePair::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if(needSize > size)
    {
        buf.Extend(needSize - size);
    } 
    return this->UnsafeStoreTo(buf.Data());
}