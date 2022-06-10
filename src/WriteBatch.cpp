#include <sharpen/WriteBatch.hpp>

#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::WriteBatch &sharpen::WriteBatch::operator=(sharpen::WriteBatch &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->actions_ = std::move(other.actions_);
    }
    return *this;
}

void sharpen::WriteBatch::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    Self::Action action;
    action.key_ = std::move(key);
    action.value_ = std::move(value);
    action.type_ = Self::ActionType::Put;
    this->actions_.emplace_back(std::move(action));
}

void sharpen::WriteBatch::Delete(sharpen::ByteBuffer key)
{
    Self::Action action;
    action.key_ = std::move(key);
    action.type_ = Self::ActionType::Delete;
    this->actions_.emplace_back(std::move(action));
}

void sharpen::WriteBatch::LoadFrom(const char *data,std::size_t size)
{
    if(size < 3)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::size_t offset{0};
    //check crc16
    std::uint16_t crc{0};
    std::memcpy(&crc,data,sizeof(crc));
    offset += 2;
    if(crc != sharpen::Crc16(data + offset,size - offset))
    {
        throw sharpen::DataCorruptionException("write batch corruption");
    }
    //number of operations
    sharpen::Varuint64 builder{data + offset,size - offset};
    offset += builder.ComputeSize();
    std::size_t num{sharpen::IntCast<std::size_t>(builder.Get())};
    //operations
    for (std::size_t i = 0; i != num; ++i)
    {
        //type
        if(size < offset)
        {
            throw sharpen::DataCorruptionException("write batch corruption");
        }
        Self::ActionType realType{Self::ActionType::Put};
        char type{data[offset]};
        offset += 1;
        if(type > 2)
        {
            throw sharpen::DataCorruptionException("write batch corruption");
        }
        else if(type == 1)
        {
            realType = Self::ActionType::Delete;
        }
        //key size
        if(size < offset)
        {
            throw sharpen::DataCorruptionException("write batch corruption");
        }
        builder.Set(data + offset,size - offset);
        offset += builder.ComputeSize();
        std::size_t keySize{sharpen::IntCast<std::size_t>(builder.Get())};
        //key
        if(size < offset + keySize)
        {
            throw sharpen::DataCorruptionException("write batch corruption");
        }
        sharpen::ByteBuffer key{keySize};
        std::memcpy(key.Data(),data + offset,keySize);
        offset += keySize;

        Self::Action action;
        action.type_ = realType;
        action.key_ = std::move(key);

        if(realType == Self::ActionType::Put)
        {
            //value size
            if(size < offset)
            {
                throw sharpen::DataCorruptionException("write batch corruption");
            }
            builder.Set(data + offset,size - offset);
            std::size_t valueSize{sharpen::IntCast<std::size_t>(builder.Get())};
            offset += builder.ComputeSize();
            //value
            if(size < offset)
            {
                throw sharpen::DataCorruptionException("write batch corruption");
            }
            sharpen::ByteBuffer value{valueSize};
            std::memcpy(value.Data(),data + offset,valueSize);
            action.value_ = std::move(value);
        }
        this->actions_.emplace_back(std::move(action));
    }
}

void sharpen::WriteBatch::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data(),buf.GetSize() - offset);
}

std::size_t sharpen::WriteBatch::ComputeSize() const noexcept
{
    std::size_t size{2};
    {
        sharpen::Varuint64 builder{this->actions_.size()};
        size += builder.ComputeSize();
    }
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size += 1;
        sharpen::Varuint64 builder{begin->key_.GetSize()};
        size += builder.ComputeSize();
        size += begin->key_.GetSize();
        if(begin->type_ == Self::ActionType::Put)
        {
            builder.Set(begin->value_.GetSize());
            size += builder.ComputeSize();
            size += begin->value_.GetSize();
        }
    }
    return size;
}

std::size_t sharpen::WriteBatch::UnsafeStoreTo(char *data) const
{
    std::size_t offset{2};
    //number
    sharpen::Varuint64 builder{this->actions_.size()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //operations
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        //type
        data[offset] = begin->type_ == Self::ActionType::Put ? 0:1;
        offset += 1;
        //key size
        builder.Set(begin->key_.GetSize());
        size = builder.ComputeSize();
        std::memcpy(data + offset,builder.Data(),size);
        offset += size;
        //key
        std::memcpy(data + offset,begin->key_.Data(),begin->key_.GetSize());
        offset += begin->key_.GetSize();
        if(begin->type_ == Self::ActionType::Put)
        {
            //value size
            builder.Set(begin->value_.GetSize());
            size = builder.ComputeSize();
            std::memcpy(data + offset,builder.Data(),size);
            offset += size;
            //value
            std::memcpy(data + offset,begin->value_.Data(),begin->value_.GetSize());
            offset += begin->value_.GetSize();
        }
    }
    //crc16
    std::uint16_t crc{sharpen::Crc16(data + sizeof(crc),offset - sizeof(crc))};
    std::memcpy(data,&crc,sizeof(crc));
    return offset;
}

std::size_t sharpen::WriteBatch::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::WriteBatch::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}