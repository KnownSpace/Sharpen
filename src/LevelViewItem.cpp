#include <sharpen/LevelViewItem.hpp>

#include <sharpen/Varint.hpp>

std::size_t sharpen::LevelViewItem::LoadFrom(const char *data,std::size_t size)
{
    if(size < 5)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::size_t offset{0};
    sharpen::Varuint64 builder{data + offset,size - offset};
    //load begin key size
    std::size_t beginSize{sharpen::IntCast<std::size_t>(builder.Get())};
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
    std::size_t endSize{builder.Get()};
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

std::size_t sharpen::LevelViewItem::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::LevelViewItem::UnsafeStoreTo(char *data) const noexcept
{
    assert(!this->beginKey_.Empty());
    assert(!this->endKey_.Empty());
    std::size_t offset{0};
    //store begin key size
    sharpen::Varuint64 builder{this->beginKey_.GetSize()};
    std::size_t size{builder.ComputeSize()};
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

std::size_t sharpen::LevelViewItem::ComputeSize() const noexcept
{
    assert(!this->beginKey_.Empty());
    assert(!this->endKey_.Empty());
    std::size_t size{0};
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

std::size_t sharpen::LevelViewItem::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::LevelViewItem::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    char *data = buf.Data() + offset;
    return this->UnsafeStoreTo(data);
}