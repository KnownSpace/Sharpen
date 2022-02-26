#include <sharpen/BtKeyValuePair.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::BtKeyValuePair::BtKeyValuePair(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
    :key_(std::move(key))
    ,value_(std::move(value))
{}

sharpen::Size sharpen::BtKeyValuePair::ComputeSize(const sharpen::ByteBuffer &key,const sharpen::ByteBuffer &value) noexcept
{
    sharpen::Size size{0};
    sharpen::Varuint64 builder{key.GetSize()};
    size += builder.ComputeSize();
    builder.Set(value.GetSize());
    size += builder.ComputeSize();
    size += key.GetSize();
    size += value.GetSize();
    return size;
}

sharpen::Size sharpen::BtKeyValuePair::ComputeSize() const noexcept
{
    return Self::ComputeSize(this->key_,this->value_);
}

sharpen::Size sharpen::BtKeyValuePair::UnsafeStoreTo(char *data) const
{
    sharpen::Size offset{0};
    //store key size
    sharpen::Varuint64 builder{this->key_.GetSize()};
    sharpen::Size size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
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

sharpen::Size sharpen::BtKeyValuePair::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 2)
    {
        throw std::invalid_argument("invalid buffer");
    }
    //load key size
    sharpen::Size keySize{0};
    sharpen::Size valueSize{0};
    sharpen::Size offset{0};
    sharpen::Varuint64 builder{data,size};
    offset += builder.ComputeSize();
    if (offset == size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    keySize = sharpen::IntCast<sharpen::Size>(builder.Get());
    //load value size
    builder.Set(data + offset,size - offset);
    offset += builder.ComputeSize();
    if (offset == size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    valueSize = sharpen::IntCast<sharpen::Size>(builder.Get());
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

sharpen::Size sharpen::BtKeyValuePair::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::BtKeyValuePair::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    this->UnsafeStoreTo(data);
    return needSize;
}

sharpen::Size sharpen::BtKeyValuePair::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(needSize > size)
    {
        buf.Extend(needSize - size);
    } 
    return this->UnsafeStoreTo(buf.Data());
}