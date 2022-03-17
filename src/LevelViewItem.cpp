#include <sharpen/LevelViewItem.hpp>

#include <sharpen/Varint.hpp>

sharpen::Size sharpen::LevelViewItem::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 5)
    {
        throw std::invalid_argument("invalid buffer");
    }
    sharpen::Size offset{0};
    sharpen::Varuint64 builder{data + offset,size - offset};
    //load begin key size
    sharpen::Size beginSize{sharpen::IntCast<sharpen::Size>(builder.Get())};
    if(!beginSize)
    {
        throw sharpen::DataCorruptionException("level view item corruption");
    }
    offset += builder.ComputeSize();
    //load begin key
    if(size < offset + beginSize)
    {
        throw sharpen::DataCorruptionException("level view item corruption");
    }
    sharpen::ByteBuffer beginKey{beginSize};
    std::memcpy(beginKey.Data(),data + offset,beginSize);
    offset += beginSize;
    this->beginKey_ = std::move(beginKey);
    //load end key size
    builder.Set(data + offset,size - offset);
    sharpen::Size endSize{builder.Get()};
    if(!endSize)
    {
        throw sharpen::DataCorruptionException("level view item corruption");
    }
    offset += builder.ComputeSize();
    //load end key
    if(size < offset + endSize)
    {
        throw sharpen::DataCorruptionException("level view item corruption");
    }
    sharpen::ByteBuffer endKey{endSize};
    std::memcpy(endKey.Data(),data + offset,endSize);
    offset += endSize;
    this->endKey_ = std::move(endKey);
    //load id
    if(size <= offset)
    {
        throw sharpen::DataCorruptionException("level view item corruption");
    }
    builder.Set(data + offset,size - offset);
    this->id_ = builder.Get();
    offset += builder.ComputeSize();
    return offset;
}

sharpen::Size sharpen::LevelViewItem::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::LevelViewItem::UnsafeStoreTo(char *data) const noexcept
{
    assert(!this->beginKey_.Empty());
    assert(!this->endKey_.Empty());
    sharpen::Size offset{0};
    //store begin key size
    sharpen::Varuint64 builder{this->beginKey_.GetSize()};
    sharpen::Size size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //store begin key
    std::memcpy(data + offset,this->beginKey_.Data(),this->beginKey_.GetSize());
    offset += this->beginKey_.GetSize();
    //store end key size
    builder.Set(this->endKey_.GetSize());
    size = builder.ComputeSize();
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //store end key
    std::memcpy(data + offset,this->endKey_.Data(),this->endKey_.GetSize());
    offset += this->endKey_.GetSize();
    //store id
    builder.Set(this->id_);
    size = builder.ComputeSize();
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    return offset;
}

sharpen::Size sharpen::LevelViewItem::ComputeSize() const noexcept
{
    assert(!this->beginKey_.Empty());
    assert(!this->endKey_.Empty());
    sharpen::Size size{0};
    //begin key size
    sharpen::Varuint64 builder{this->BeginKey().GetSize()};
    size += builder.ComputeSize();
    //begin key
    size += this->BeginKey().GetSize();
    //end key size
    builder.Set(this->EndKey().GetSize());
    size += builder.ComputeSize();
    //end key
    size += this->EndKey().GetSize();
    //id
    builder.Set(this->id_);
    size += builder.ComputeSize();
    return size;
}

sharpen::Size sharpen::LevelViewItem::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::LevelViewItem::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size size{buf.GetSize() - offset};
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    char *data = buf.Data() + offset;
    return this->UnsafeStoreTo(data);
}