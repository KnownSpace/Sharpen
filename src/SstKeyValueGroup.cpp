#include <sharpen/SstKeyValueGroup.hpp>

void sharpen::SstKeyValueGroup::LoadFrom(const char *data,sharpen::Size size)
{
    this->pairs_.clear();
    if(size < 3)
    {
        //empty group
        return;
    }
    sharpen::SstKeyValuePair pair;
    sharpen::Size offset{pair.LoadFrom(data,size)};
    this->pairs_.emplace_back(std::move(pair));
    while (offset != size)
    {
        try
        {
            offset += pair.LoadFrom(data + offset,size - offset);
            this->pairs_.emplace_back(std::move(pair));
        }
        catch(const std::exception& e)
        {
            this->pairs_.clear();
            throw;
        }
    }
}

void sharpen::SstKeyValueGroup::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::SstKeyValueGroup::ComputeSize() const noexcept
{
    sharpen::Size size{0};
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size += begin->ComputeSize();   
    }
    return size;
}

void sharpen::SstKeyValueGroup::InternalStoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size offset{0};
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        offset += begin->StoreTo(data + offset,size - offset);
    }
}

void sharpen::SstKeyValueGroup::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    this->InternalStoreTo(data,size);
}

void sharpen::SstKeyValueGroup::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    this->InternalStoreTo(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::SstKeyValueGroup::Iterator sharpen::SstKeyValueGroup::Find(const sharpen::ByteBuffer &key)
{
    auto begin = this->Begin();
    auto end = this->End();
    while (begin != end)
    {
        sharpen::Size size = sharpen::GetRangeSize(begin,end);
        auto mid = begin + size/2;
        if(mid->GetKey() == key)
        {
            return mid;
        }
        else if(mid->GetKey() > key)
        {
            end = mid;
        }
        else if(mid->GetKey() < key)
        {
            begin = sharpen::IteratorForward(mid,1);
        }
    }
    return begin;
}

sharpen::SstKeyValueGroup::ConstIterator sharpen::SstKeyValueGroup::Find(const sharpen::ByteBuffer &key) const
{
    auto begin = this->Begin();
    auto end = this->End();
    while (begin != end)
    {
        sharpen::Size size = sharpen::GetRangeSize(begin,end);
        auto mid = begin + size/2;
        if(mid->GetKey() == key)
        {
            return mid;
        }
        else if(mid->GetKey() > key)
        {
            end = mid;
        }
        else if(mid->GetKey() < key)
        {
            begin = sharpen::IteratorForward(mid,1);
        }
    }
    return begin;
}

bool sharpen::SstKeyValueGroup::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->Find(key);
    return ite != this->End() && ite->GetKey() == key;
}

sharpen::SstKeyValuePair &sharpen::SstKeyValueGroup::Get(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite == this->End() || ite->GetKey() != key)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return *ite;
}

const sharpen::SstKeyValuePair &sharpen::SstKeyValueGroup::Get(const sharpen::ByteBuffer &key) const
{
    auto ite = this->Find(key);
    if(ite == this->End() || ite->GetKey() != key)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return *ite;
}

void sharpen::SstKeyValueGroup::Delete(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite != this->End() && ite->GetKey() == key)
    {
        this->pairs_.erase(ite);
    }
}

void sharpen::SstKeyValueGroup::Put(sharpen::SstKeyValuePair pair)
{
    auto ite = this->Find(pair.GetKey());
    if(ite != this->End())
    {
        if(ite->GetKey() == pair.GetKey())
        {
            *ite = std::move(pair);
            return;
        }
        this->pairs_.emplace(ite,std::move(pair));
        return;
    }
    this->pairs_.emplace_back(std::move(pair));
}

void sharpen::SstKeyValueGroup::Update(const sharpen::ByteBuffer &oldKey,sharpen::SstKeyValuePair pair)
{
    this->Delete(oldKey);
    this->Put(std::move(pair));
}