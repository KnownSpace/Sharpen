#include <sharpen/SstKeyValueGroup.hpp>

sharpen::SstKeyValueGroup::SstKeyValueGroup()
    :pairs_()
    ,comp_(nullptr)
{}

void sharpen::SstKeyValueGroup::LoadFrom(const char *data,std::size_t size)
{
    this->pairs_.clear();
    if(size < 3)
    {
        //empty group
        return;
    }
    sharpen::SstKeyValuePair pair;
    std::size_t offset{pair.LoadFrom(data,size)};
    this->pairs_.emplace_back(std::move(pair));
    while (offset != size)
    {
        try
        {
            offset += pair.LoadFrom(data + offset,size - offset);
            pair.SetSharedKey(this->First().GetKey().Data(),pair.GetSharedKeySize());
            this->pairs_.emplace_back(std::move(pair));
        }
        catch(const std::exception&)
        {
            this->pairs_.clear();
            throw;
        }
    }
}

void sharpen::SstKeyValueGroup::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::SstKeyValueGroup::ComputeSize() const noexcept
{
    std::size_t size{0};
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size += begin->ComputeSize();   
    }
    return size;
}

std::size_t sharpen::SstKeyValueGroup::UnsafeStoreTo(char *data) const
{
    std::size_t offset{0};
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        offset += begin->UnsafeStoreTo(data + offset);
    }
    return offset;
}

std::size_t sharpen::SstKeyValueGroup::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::SstKeyValueGroup::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
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

bool sharpen::SstKeyValueGroup::WarppedComp(Comparator comp,const sharpen::SstKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept
{
    if(!comp)
    {
        return Self::Comp(pair,key);
    }
    return comp(pair.GetKey(),key) == -1;
}

bool sharpen::SstKeyValueGroup::Comp(const sharpen::SstKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept
{
    return pair.GetKey() < key;
}

std::int32_t sharpen::SstKeyValueGroup::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if (!this->comp_)
    {
        return left.CompareWith(right);
    }
    return this->comp_(left,right);
}

sharpen::SstKeyValueGroup::Iterator sharpen::SstKeyValueGroup::Find(const sharpen::ByteBuffer &key)
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstKeyValuePair&,const sharpen::ByteBuffer &);
        return std::lower_bound(this->pairs_.begin(),this->pairs_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::SstKeyValuePair&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->pairs_.begin(),this->pairs_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::SstKeyValueGroup::ConstIterator sharpen::SstKeyValueGroup::Find(const sharpen::ByteBuffer &key) const
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstKeyValuePair&,const sharpen::ByteBuffer &);
        return std::lower_bound(this->pairs_.begin(),this->pairs_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::SstKeyValuePair&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->pairs_.begin(),this->pairs_.end(),key,static_cast<FnPtr>(&Self::Comp));
}

bool sharpen::SstKeyValueGroup::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->Find(key);
    return ite != this->End() && this->CompKey(ite->GetKey(),key) == 0;
}

sharpen::SstKeyValuePair &sharpen::SstKeyValueGroup::Get(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite == this->End() || this->CompKey(ite->GetKey(),key) != 0)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return *ite;
}

const sharpen::SstKeyValuePair &sharpen::SstKeyValueGroup::Get(const sharpen::ByteBuffer &key) const
{
    auto ite = this->Find(key);
    if(ite == this->End() || this->CompKey(ite->GetKey(),key) != 0)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return *ite;
}

void sharpen::SstKeyValueGroup::Delete(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite != this->End() && this->CompKey(ite->GetKey(),key) == 0)
    {
        this->pairs_.erase(ite);
    }
}

std::pair<std::uint64_t,std::uint64_t> sharpen::SstKeyValueGroup::ComputeKeySizes(const sharpen::ByteBuffer &key) const noexcept
{
    std::size_t sharedSize{0};
    for (std::size_t count = (std::min)(this->First().GetKey().GetSize(),key.GetSize()); sharedSize != count && this->First().GetKey()[sharedSize] == key[sharedSize]; ++sharedSize)
    {}
    std::uint64_t uniquedSize = key.GetSize() - sharedSize;
    return {sharedSize,uniquedSize};
}

bool sharpen::SstKeyValueGroup::TryPut(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        //key already exist
        if(this->CompKey(ite->GetKey(),key) == 0)
        {
            ite->Value() = std::move(value);
            return true;
        }
        //not last key
        auto sizes = this->ComputeKeySizes(key);
        if(!sizes.first)
        {
            return false;
        }
        //first key
        if(ite == this->Begin())
        {
            this->pairs_.emplace(ite,0,key.GetSize(),std::move(key),std::move(value));
            //re-compute key sizes
            for (auto begin = sharpen::IteratorForward(this->pairs_.begin(),1); begin != this->pairs_.end(); ++begin)
            {
                sizes = this->ComputeKeySizes(begin->GetKey());
                begin->SetSharedKeySize(sizes.first);
                begin->SetUniquedKeySize(sizes.second);
            }
            return true;
        }
        ite = sharpen::IteratorForward(ite,1);
        if(ite == this->End())
        {
            this->pairs_.emplace_back(sizes.first,sizes.second,std::move(key),std::move(value));
        }
        else
        {
            this->pairs_.emplace(ite,sizes.first,sizes.second,std::move(key),std::move(value));
        }
        return true;
    }
    //last key
    if(this->Empty())
    {
        std::size_t keySize{key.GetSize()};
        this->pairs_.emplace_back(0,keySize,std::move(key),std::move(value));
        return true;
    }
    auto sizes = this->ComputeKeySizes(key);
    if(!sizes.first)
    {
        return false;
    }
    this->pairs_.emplace_back(sizes.first,sizes.second,std::move(key),std::move(value));
    return true;
}

void sharpen::SstKeyValueGroup::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    if(!this->TryPut(std::move(key),std::move(value)))
    {
        throw std::invalid_argument("cannot insert key to group");
    }
}

bool sharpen::SstKeyValueGroup::CheckKey(const sharpen::ByteBuffer &key) const noexcept
{
    return this->Empty() || this->ComputeKeySizes(key).first;
}