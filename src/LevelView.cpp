#include <sharpen/LevelView.hpp>

#include <algorithm>

#include <sharpen/Varint.hpp>

bool sharpen::LevelView::Comp(const sharpen::LevelViewItem &item,const sharpen::ByteBuffer &key) noexcept
{
    return item.EndKey() < key;
}

bool sharpen::LevelView::WarpComp(Comparator comp,const sharpen::LevelViewItem &item,const sharpen::ByteBuffer &key) noexcept
{
    if(comp)
    {
        return comp(item.EndKey(),key) == -1;
    }
    return Self::Comp(item,key);
}

std::int32_t sharpen::LevelView::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::LevelView::Iterator sharpen::LevelView::Find(const sharpen::ByteBuffer &key) noexcept
{
    if(this->comp_)
    {
        using Warpped = bool(*)(Comparator,const sharpen::LevelViewItem&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->Begin(),this->End(),key,std::bind(static_cast<Warpped>(&Self::WarpComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::LevelViewItem&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->Begin(),this->End(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::LevelView::ConstIterator sharpen::LevelView::Find(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->comp_)
    {
        using Warpped = bool(*)(Comparator,const sharpen::LevelViewItem&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->Begin(),this->End(),key,std::bind(static_cast<Warpped>(&Self::WarpComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::LevelViewItem&,const sharpen::ByteBuffer&);
    return std::lower_bound(this->Begin(),this->End(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::LevelView::LevelView(std::uint64_t id)
    :id_(id)
    ,items_()
    ,comp_(nullptr)
{}

std::size_t sharpen::LevelView::LoadFrom(const char *data,std::size_t size)
{
    if (size < 1)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::size_t offset{0};
    //load item count
    sharpen::Varuint64 builder{data + offset,size - offset};
    std::size_t itemSize{builder.Get()};
    offset += builder.ComputeSize();
    this->items_.reserve(itemSize);
    //load items
    for (std::size_t i = 0; i != itemSize; ++i)
    {
        sharpen::LevelViewItem item;
        try
        {
            offset += item.LoadFrom(data + offset,size - offset);
            this->items_.emplace_back(std::move(item));
        }
        catch(const std::exception&)
        {
            this->items_.clear();
            throw;
        }
    }
    return offset;
}

std::size_t sharpen::LevelView::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::LevelView::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    //store item count
    sharpen::Varuint64 builder{this->items_.size()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data + offset,builder.Data(),size);
    offset += size;
    //store items
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size = begin->UnsafeStoreTo(data + offset);
        offset += size;
    }
    return offset;
}

std::size_t sharpen::LevelView::ComputeSize() const noexcept
{
    std::size_t offset{0};
    //store item count
    sharpen::Varuint64 builder{this->items_.size()};
    std::size_t size{builder.ComputeSize()};
    offset += size;
    //store items
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        size = begin->ComputeSize();
        offset += size;
    }
    return offset;
}

std::size_t sharpen::LevelView::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::LevelView::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
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

sharpen::Optional<std::uint64_t> sharpen::LevelView::FindId(const sharpen::ByteBuffer &key) const
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        std::int32_t r{this->CompKey(ite->BeginKey(),key)};
        if(r != 1)
        {
            return ite->GetId();
        }
    }
    return sharpen::EmptyOpt;
}

bool sharpen::LevelView::IsNotOverlapped(const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const noexcept
{
    auto ite = this->Find(beginKey);
    if(ite != this->End())
    {
        std::int32_t r{this->CompKey(ite->BeginKey(),beginKey)};
        if(r != 1)
        {
            return false;
        }
        auto it = this->Find(endKey);
        if(it != ite)
        {
            return false;
        }
        r = this->CompKey(it->BeginKey(),endKey);
        if(r != 1)
        {
            return false;
        }
    }
    return true;
}

bool sharpen::LevelView::TryPut(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,std::uint64_t id)
{
    auto ite = this->Find(beginKey);
    if(ite != this->End())
    {
        std::int32_t r{this->CompKey(ite->BeginKey(),beginKey)};
        if(r != 1)
        {
            return false;
        }
        auto it = this->Find(endKey);
        if(it != ite)
        {
            return false;
        }
        r = this->CompKey(it->BeginKey(),endKey);
        if(r != 1)
        {
            return false;
        }
        this->items_.emplace(ite,std::move(beginKey),std::move(endKey),id);
        return true;
    }
    this->items_.emplace_back(std::move(beginKey),std::move(endKey),id);
    return true;
}

void sharpen::LevelView::Put(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,std::uint64_t id)
{
    bool r{this->TryPut(std::move(beginKey),std::move(endKey),id)};
    if(!r)
    {
        throw std::invalid_argument("item overlapped with other items");
    }   
}

void sharpen::LevelView::Delete(std::uint64_t id)
{
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        if(begin->GetId() == id)
        {
            this->items_.erase(begin);
            return;
        }   
    }
}