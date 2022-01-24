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

void sharpen::WriteBatch::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 3)
    {
        throw std::invalid_argument("invalid buffer");
    }
    sharpen::Size offset{0};
    //check crc16
    sharpen::Uint16 crc{0};
    std::memcpy(&crc,data,sizeof(crc));
    offset += 2;
    if(crc != sharpen::Crc16(data + offset,size - offset))
    {
        throw sharpen::DataCorruptionException("write batch corruption");
    }
    //number of operations
    sharpen::Varuint64 builder{data + offset,size - offset};
    offset += builder.ComputeSize();
    sharpen::Size num{sharpen::IntCast<sharpen::Size>(builder.Get())};
    //operations
    for (sharpen::Size i = 0; i != num; ++i)
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
        sharpen::Size keySize{sharpen::IntCast<sharpen::Size>(builder.Get())};
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
            sharpen::Size valueSize{sharpen::IntCast<sharpen::Size>(builder.Get())};
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

void sharpen::WriteBatch::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data(),buf.GetSize() - offset);
}

sharpen::Size sharpen::WriteBatch::ComputeSize() const noexcept
{
    sharpen::Size size{2};
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

sharpen::Size sharpen::WriteBatch::UnsafeStoreTo(char *data) const
{
    sharpen::Size offset{2};
    //number
    sharpen::Varuint64 builder{this->actions_.size()};
    sharpen::Size size{builder.ComputeSize()};
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
    sharpen::Uint16 crc{sharpen::Crc16(data + sizeof(crc),offset - sizeof(crc))};
    std::memcpy(data,&crc,sizeof(crc));
    return offset;
}

sharpen::Size sharpen::WriteBatch::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::WriteBatch::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}