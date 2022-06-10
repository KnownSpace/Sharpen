#include <sharpen/SstKeyValuePair.hpp>

sharpen::SstKeyValuePair::SstKeyValuePair()
    :sharedSize_(0)
    ,uniquedSize_(0)
    ,key_()
    ,value_()
{}

sharpen::SstKeyValuePair::SstKeyValuePair(std::uint64_t sharedSize,std::uint64_t uniquedSize,sharpen::ByteBuffer key,sharpen::ByteBuffer value)
    :sharedSize_(sharedSize)
    ,uniquedSize_(uniquedSize)
    ,key_(std::move(key))
    ,value_(std::move(value))
{}

std::size_t sharpen::SstKeyValuePair::LoadFrom(const char *data,std::size_t size)
{
    if(size < 3)
    {
        throw std::invalid_argument("invalid buffer");
    }
    sharpen::Varuint64 builder{data,size};
    std::size_t offset{builder.ComputeSize()};
    if(offset > size)
    {
        throw std::invalid_argument("invalid buffer");
    }
    this->sharedSize_ = builder.Get();
    builder.Set(data + offset,size - offset);
    offset += builder.ComputeSize();
    if(offset > size)
    {
        this->sharedSize_ = 0;
        throw std::invalid_argument("invalid buffer");
    }
    this->uniquedSize_ = builder.Get();
    std::size_t keySize{sharpen::IntCast<std::size_t>(this->uniquedSize_ + this->sharedSize_)};
    this->key_.ExtendTo(keySize);
    if(this->uniquedSize_)
    {
        if(offset + this->uniquedSize_ > size)
        {
            this->sharedSize_ = 0;
            this->uniquedSize_ = 0;
            throw std::invalid_argument("invalid buffer");
        }
        std::memcpy(this->key_.Data() + this->sharedSize_,data + offset,this->uniquedSize_);
        offset = sharpen::IntCast<std::size_t>(this->uniquedSize_ + offset);
    }
    builder.Set(data + offset,size - offset);
    this->value_.ExtendTo(sharpen::IntCast<std::size_t>(builder.Get()));
    offset += builder.ComputeSize();
    if (builder.Get())
    {
        if(offset + builder.Get() > size)
        {
            this->sharedSize_ = 0;
            this->uniquedSize_ = 0;
            this->key_.Clear();
            throw std::invalid_argument("invalid buffer");
        }
        std::memcpy(this->value_.Data(),data + offset,builder.Get());
        offset = sharpen::IntCast<std::size_t>(builder.Get() + offset);
    }
    return offset;
}

std::size_t sharpen::SstKeyValuePair::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::SstKeyValuePair::ComputeSize() const noexcept
{
    sharpen::Varuint64 builder{this->sharedSize_};
    std::size_t size{builder.ComputeSize()};
    builder.Set(this->uniquedSize_);
    size += builder.ComputeSize();
    assert(this->uniquedSize_ + this->sharedSize_ == this->key_.GetSize());
    size = sharpen::IntCast<std::size_t>(this->uniquedSize_ + size);
    builder.Set(this->value_.GetSize());
    size += builder.ComputeSize();
    size += this->value_.GetSize();
    return size;
}

std::size_t sharpen::SstKeyValuePair::UnsafeStoreTo(char *data) const
{
    //store shared size
    sharpen::Varuint64 builder{this->sharedSize_};
    std::size_t offset{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),offset);
    //store uniqued size
    builder.Set(this->uniquedSize_);
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    assert(this->uniquedSize_ + this->sharedSize_ == this->key_.GetSize());
    //store uniqued key
    if(this->uniquedSize_)
    {
        std::memcpy(data + offset,this->key_.Data() + this->sharedSize_,sharpen::IntCast<std::size_t>(this->uniquedSize_));
    }
    offset = sharpen::IntCast<std::size_t>(this->uniquedSize_ + offset);
    //store value size
    builder.Set(this->value_.GetSize());
    size = builder.ComputeSize();
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //store value
    if (!this->value_.Empty())
    {
        std::memcpy(data + offset,this->value_.Data(),this->value_.GetSize());   
    }
    offset += this->value_.GetSize();
    return offset;
}

std::size_t sharpen::SstKeyValuePair::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(needSize > size)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::SstKeyValuePair::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if(needSize > size)
    {
        buf.Extend(needSize - size);
    } 
    return this->UnsafeStoreTo(buf.Data() + offset);
}