#include <sharpen/SstDataBlock.hpp>

void sharpen::SstDataBlock::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 3)
    {
        throw std::invalid_argument("invalid buffer");
    }
    sharpen::Uint16 crc{0};
    std::memcpy(&crc,data,sizeof(crc));
    sharpen::Varuint64 groups{0};
    sharpen::Size offset{sizeof(crc)};
    //checksum
    if(crc != sharpen::Crc16(data + offset,size - offset))
    {
        throw sharpen::DataCorruptionException("data block corruption");
    }
    //number of groups
    groups.Set(data + offset,size - offset);
    offset += groups.ComputeSize();
    //load key value groups index
    sharpen::Size number{sharpen::IntCast<sharpen::Size>(groups.Get())};
    if(number*sizeof(sharpen::Uint32) + offset > size)
    {
        throw sharpen::DataCorruptionException("data block corruption");
    }
    std::vector<std::pair<sharpen::Size,sharpen::Size>> indexs;
    indexs.reserve(number - 1);
    {
        sharpen::Uint32 off{0};
        for (sharpen::Size i = 0,count = number - 1; i < count; ++i)
        {
            std::memcpy(&off,data + offset,sizeof(off));
            sharpen::Size begin{off};
            //check error
            if(begin + sizeof(off)*number > size)
            {
                throw sharpen::DataCorruptionException("data block corruption");
            }
            offset += sizeof(off);
            std::memcpy(&off,data + offset,sizeof(off));
            offset += sizeof(off);
            sharpen::Size end{off};
            //check error
            if(end+ sizeof(off)*number > size || end < begin || end - begin + sizeof(off)*number > size)
            {
                throw sharpen::DataCorruptionException("data block corruption");
            }
            indexs.emplace_back(begin,end - begin);
        }
        std::memcpy(&off,data + offset,sizeof(off));
        sharpen::Size begin{off};
        offset += sizeof(off);
        indexs.emplace_back(begin,size - begin);
    }
    //load key value groups
    sharpen::SstKeyValueGroup group;
    for (auto begin = indexs.begin(),end = indexs.end(); begin != end; ++begin)
    {
        try
        {
            group.LoadFrom(data + begin->first,begin->second);
            this->groups_.emplace_back(std::move(group));
        }
        catch(const std::exception&)
        {
            this->groups_.clear();
            throw;
        }
    }
}

void sharpen::SstDataBlock::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() > offset);
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::SstDataBlock::ComputeSize() const noexcept
{
    sharpen::Size size{sizeof(sharpen::Uint16)};
    sharpen::Varuint64 builder{this->groups_.size()};
    size += builder.ComputeSize();
    size += builder.Get()*sizeof(sharpen::Uint32);
    for (auto begin = this->groups_.cbegin(),end = this->groups_.cend(); begin != end; ++begin)
    {
        size += begin->ComputeSize();
    }
    return size;
}

sharpen::Size sharpen::SstDataBlock::UnsafeStoreTo(char *data) const
{
    sharpen::Uint16 crc{0};
    sharpen::Varuint64 builder{this->groups_.size()};
    //store number
    sharpen::Size offset{builder.ComputeSize()};
    std::memcpy(data + sizeof(crc),builder.Data(),offset);
    offset += sizeof(crc);
    //store key value offsets and groups
    sharpen::Size last{offset + sizeof(sharpen::Uint32)*this->groups_.size()};
    for (sharpen::Size i = 0,count = this->groups_.size(); i != count; ++i)
    {
        sharpen::Size kvSize{this->groups_[i].UnsafeStoreTo(data + offset)};
        sharpen::Uint32 off{sharpen::IntCast<sharpen::Uint32>(last)};
        std::memcpy(data + offset + i*sizeof(off),&off,sizeof(off));
        last += kvSize;
    }
    //store checksum
    crc = sharpen::Crc16(data + sizeof(crc),last - sizeof(crc));
    std::memcpy(data,&crc,sizeof(crc));
    return last;
}

sharpen::Size sharpen::SstDataBlock::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::SstDataBlock::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() > offset);
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(needSize > size)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data());
}

bool sharpen::SstDataBlock::Comp(const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept
{
    assert(!group.Empty());
    return group.Last().GetKey() < key;
}

sharpen::SstDataBlock::Iterator sharpen::SstDataBlock::FindGroup(const sharpen::ByteBuffer &key)
{
    using FnPtr = bool(*)(const sharpen::SstKeyValueGroup&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->groups_.begin(),this->groups_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::SstDataBlock::ConstIterator sharpen::SstDataBlock::FindGroup(const sharpen::ByteBuffer &key) const
{
    using FnPtr = bool(*)(const sharpen::SstKeyValueGroup&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->groups_.begin(),this->groups_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

bool sharpen::SstDataBlock::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->FindGroup(key);
    return ite != this->End() && ite->Exist(key);
}

void sharpen::SstDataBlock::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    auto ite = this->FindGroup(key);
    if(ite != this->groups_.end())
    {
        //check and insert key
        if(ite->CheckKey(key))
        {
            bool succ = ite->TryPut(std::move(key),std::move(value));
            assert(succ);
            static_cast<void>(succ);
            return;
        }
        //need to create a new group
        sharpen::SstKeyValueGroup group;
        group.Put(std::move(key),std::move(value));
        this->groups_.emplace(ite,std::move(group));
        return;
    }
    sharpen::SstKeyValueGroup group;
    group.Put(std::move(key),std::move(value));
    this->groups_.emplace_back(std::move(group));
}

void sharpen::SstDataBlock::Delete(const sharpen::ByteBuffer &key)
{
    auto ite = this->FindGroup(key);
    if(ite != this->End())
    {
        ite->Delete(key);
        if (ite->Empty())
        {
            this->groups_.erase(ite);
        }
    }
}

sharpen::Size sharpen::SstDataBlock::ComputeKeyCount() const noexcept
{
    sharpen::Size size{0};
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size += begin->GetSize();
    }
    return size;
}