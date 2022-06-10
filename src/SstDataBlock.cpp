#include <sharpen/SstDataBlock.hpp>

sharpen::SstDataBlock::SstDataBlock()
    :groups_()
    ,comp_(nullptr)
{}

void sharpen::SstDataBlock::LoadFrom(const char *data, std::size_t size)
{
    if (size < 3)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::uint16_t crc{0};
    std::memcpy(&crc, data, sizeof(crc));
    sharpen::Varuint64 groups{0};
    std::size_t offset{sizeof(crc)};
    //checksum
    if (crc != sharpen::Crc16(data + offset, size - offset))
    {
        throw sharpen::DataCorruptionException("data block corruption");
    }
    //number of groups
    groups.Set(data + offset, size - offset);
    offset += groups.ComputeSize();
    //load key value groups index
    std::size_t number{sharpen::IntCast<std::size_t>(groups.Get())};
    if (number * sizeof(std::uint64_t) + offset > size)
    {
        throw sharpen::DataCorruptionException("data block corruption");
    }
    std::vector<std::pair<std::size_t, std::size_t>> indexs;
    indexs.reserve(number);
    {
        std::uint64_t off{0};
        for (std::size_t i = 1, count = number; i < count; ++i)
        {
            std::memcpy(&off, data + offset, sizeof(off));
            std::size_t begin{sharpen::IntCast<std::size_t>(off)};
            //check error
            if (begin > size)
            {
                throw sharpen::DataCorruptionException("data block corruption");
            }
            offset += sizeof(off);
            std::memcpy(&off, data + offset, sizeof(off));
            std::size_t end{off};
            //check error
            if (end > size || end < begin)
            {
                throw sharpen::DataCorruptionException("data block corruption");
            }
            indexs.emplace_back(begin, end - begin);
        }
        std::memcpy(&off, data + offset, sizeof(off));
        std::size_t begin{off};
        offset += sizeof(off);
        if (begin > size)
        {
            throw sharpen::DataCorruptionException("data block corruption");
        }
        indexs.emplace_back(begin, size - begin);
    }
    //load key value groups
    sharpen::SstKeyValueGroup group;
    this->groups_.clear();
    for (auto begin = indexs.begin(), end = indexs.end(); begin != end; ++begin)
    {
        try
        {
            group.LoadFrom(data + begin->first, begin->second);
            group.SetComparator(this->GetComparator());
            this->groups_.emplace_back(std::move(group));
        }
        catch (const std::exception &)
        {
            this->groups_.clear();
            throw;
        }
    }
}

void sharpen::SstDataBlock::LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data() + offset, buf.GetSize() - offset);
}

std::size_t sharpen::SstDataBlock::ComputeSize() const noexcept
{
    std::size_t size{sizeof(std::uint16_t)};
    sharpen::Varuint64 builder{this->groups_.size()};
    size += builder.ComputeSize();
    size += builder.Get() * sizeof(std::uint64_t);
    for (auto begin = this->groups_.cbegin(), end = this->groups_.cend(); begin != end; ++begin)
    {
        size += begin->ComputeSize();
    }
    return size;
}

std::size_t sharpen::SstDataBlock::UnsafeStoreTo(char *data) const noexcept
{
    std::uint16_t crc{0};
    sharpen::Varuint64 builder{this->groups_.size()};
    //store number
    std::size_t offset{builder.ComputeSize()};
    std::memcpy(data + sizeof(crc), builder.Data(), offset);
    offset += sizeof(crc);
    //store key value offsets and groups
    std::size_t last{offset + sizeof(std::uint64_t) * this->groups_.size()};
    for (std::size_t i = 0, count = this->groups_.size(); i != count; ++i)
    {
        std::size_t kvSize{this->groups_[i].UnsafeStoreTo(data + last)};
        std::uint64_t off{last};
        std::memcpy(data + offset + i * sizeof(off), &off, sizeof(off));
        last += kvSize;
    }
    //store checksum
    crc = sharpen::Crc16(data + sizeof(crc), last - sizeof(crc));
    std::memcpy(data, &crc, sizeof(crc));
    return last;
}

std::size_t sharpen::SstDataBlock::StoreTo(char *data, std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::SstDataBlock::StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if (needSize > size)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

bool sharpen::SstDataBlock::WarppedComp(Comparator comp,const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept
{
    if(comp)
    {
        return comp(group.Last().GetKey(),key) == -1;
    }
    return Self::Comp(group,key);
}

bool sharpen::SstDataBlock::Comp(const sharpen::SstKeyValueGroup &group, const sharpen::ByteBuffer &key) noexcept
{
    assert(!group.Empty());
    return group.Last().GetKey() < key;
}

std::int32_t sharpen::SstDataBlock::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::SstDataBlock::Iterator sharpen::SstDataBlock::FindGroup(const sharpen::ByteBuffer &key)
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstKeyValueGroup&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->groups_.begin(),this->groups_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool (*)(const sharpen::SstKeyValueGroup &, const sharpen::ByteBuffer &);
    return std::lower_bound(this->groups_.begin(), this->groups_.end(), key, static_cast<FnPtr>(&Self::Comp));
}

sharpen::SstDataBlock::ConstIterator sharpen::SstDataBlock::FindGroup(const sharpen::ByteBuffer &key) const
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::SstKeyValueGroup&,const sharpen::ByteBuffer&);
        return std::lower_bound(this->groups_.begin(),this->groups_.end(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool (*)(const sharpen::SstKeyValueGroup &, const sharpen::ByteBuffer &);
    return std::lower_bound(this->groups_.begin(), this->groups_.end(), key, static_cast<FnPtr>(&Self::Comp));
}

sharpen::SstDataBlock::Iterator sharpen::SstDataBlock::FindInsertGroup(const sharpen::ByteBuffer &key)
{
    auto ite = this->FindGroup(key);
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite, 1);
    }
    else if(ite != this->Begin() && this->CompKey(key,ite->First().GetKey()) == -1 && !ite->CheckKey(key))
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::SstDataBlock::ConstIterator sharpen::SstDataBlock::FindInsertGroup(const sharpen::ByteBuffer &key) const
{
    auto ite = this->FindGroup(key);
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite, 1);
    }
    else if(ite != this->Begin() && this->CompKey(key,ite->First().GetKey()) == -1 && !ite->CheckKey(key))
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::ExistStatus sharpen::SstDataBlock::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->FindGroup(key);
    if(ite != this->End() && ite->Exist(key))
    {
        if (!ite->Get(key).Value().Empty())
        {
            return sharpen::ExistStatus::Exist;   
        }
        return sharpen::ExistStatus::Deleted;
    }
    return sharpen::ExistStatus::NotExist;
}

void sharpen::SstDataBlock::Put(sharpen::ByteBuffer key, sharpen::ByteBuffer value)
{
    if(this->groups_.capacity() < this->groups_.size() + 1)
    {
        this->groups_.reserve(this->groups_.size() + 1);
    }
    auto ite = this->FindInsertGroup(key);
    if (ite != this->groups_.end())
    {
        //check and insert key
        if (ite->CheckKey(key))
        {
            if (ite->GetSize() >= maxKeyPerGroups_)
            {
                //div this group to 2 groups
                sharpen::SstKeyValueGroup group;
                auto begin = sharpen::IteratorForward(ite->Begin(), maxKeyPerGroups_);
                auto end = ite->End();
                group.Reserve(sharpen::GetRangeSize(begin, end) + 1);
                for (auto i = begin; i != end; ++i)
                {
                    sharpen::ByteBuffer v{std::move(i->Value())};
                    sharpen::ByteBuffer k{std::move(*i).MoveKey()};
                    group.Put(std::move(k),std::move(v));
                }
                bool succ = group.TryPut(std::move(key), std::move(value));
                assert(succ);
                static_cast<void>(succ);
                ite = sharpen::IteratorForward(ite, 1);
                if (ite == this->groups_.end())
                {
                    this->groups_.emplace_back(std::move(group));
                    return;
                }
                this->groups_.emplace(ite, std::move(group));
            }
            else
            {
                bool succ = ite->TryPut(std::move(key), std::move(value));
                assert(succ);
                static_cast<void>(succ);
            }
            return;
        }
        //need to create a new group
        sharpen::SstKeyValueGroup group;
        group.SetComparator(this->GetComparator());
        group.Put(std::move(key), std::move(value));
        if(this->CompKey(group.First().GetKey(),ite->First().GetKey()) == 1)
        {
            ite = sharpen::IteratorForward(ite, 1);
        }
        if (ite == this->End())
        {
            this->groups_.emplace_back(std::move(group));
        }
        else
        {
            this->groups_.emplace(ite, std::move(group));
        }
        return;
    }
    sharpen::SstKeyValueGroup group;
    group.SetComparator(this->GetComparator());
    group.Put(std::move(key), std::move(value));
    this->groups_.emplace_back(std::move(group));
}

void sharpen::SstDataBlock::Delete(sharpen::ByteBuffer key)
{
    this->Put(std::move(key),sharpen::ByteBuffer{});
}

std::size_t sharpen::SstDataBlock::ComputeKeyCount() const noexcept
{
    std::size_t size{0};
    for (auto begin = this->Begin(), end = this->End(); begin != end; ++begin)
    {
        size += begin->GetSize();
    }
    return size;
}

sharpen::ByteBuffer &sharpen::SstDataBlock::Get(const sharpen::ByteBuffer &key)
{
    auto ite = this->FindGroup(key);
    if (ite == this->groups_.end())
    {
        throw std::out_of_range("key doesn't exists(group not found)");
    }
    return ite->GetValue(key);
}

const sharpen::ByteBuffer &sharpen::SstDataBlock::Get(const sharpen::ByteBuffer &key) const
{
    auto ite = this->FindGroup(key);
    if (ite == this->groups_.end())
    {
        throw std::out_of_range("key doesn't exists(group not found)");
    }
    return ite->GetValue(key);
}

void sharpen::SstDataBlock::MergeWith(sharpen::SstDataBlock block, bool reserveCurrent)
{
    this->MergeWith(std::make_move_iterator(&block),std::make_move_iterator(&block + 1),reserveCurrent);
}

sharpen::SstDataBlock sharpen::SstDataBlock::Split()
{
    sharpen::SstDataBlock block;
    std::size_t keyCount{this->ComputeKeyCount() / 2};
    std::size_t movedKey{0};
    auto begin = this->ReverseBegin();
    auto end = this->ReverseEnd();
    for (; movedKey < keyCount && begin != end; ++begin)
    {
        block.groups_.emplace_back(std::move(*begin));
        movedKey += block.groups_.back().GetSize();
    }
    std::size_t moved{sharpen::GetRangeSize(this->ReverseBegin(), begin)};
    this->groups_.erase(sharpen::IteratorBackward(this->End(), moved), this->End());
    return block;
}

bool sharpen::SstDataBlock::IsAtomic() const noexcept
{
    std::size_t size{this->ComputeKeyCount()/2};
    for (auto begin = this->Begin(), end = this->End(); begin != end; ++begin)
    {
        if (begin->GetSize() > size)
        {
            return true;
        }
    }
    return false;
}

bool sharpen::SstDataBlock::IsOverlapped(const Self &other) const noexcept
{
    if (this->Empty() || other.Empty())
    {
        return false;
    }
    if(this->LastKey() < other.FirstKey())
    {
        return false;
    }
    if(this->FirstKey() > other.LastKey())
    {
        return false;
    }
    return true;
}

void sharpen::SstDataBlock::EraseDeleted() noexcept
{
    for (auto begin = this->Begin(); begin != this->End();)
    {
        for (auto keyBegin = begin->Begin(); keyBegin != begin->End();)
        {
            if(keyBegin->Value().Empty())
            {
                keyBegin = begin->Erase(keyBegin);
            }
            else
            {
                ++keyBegin;
            }   
        }
        if (begin->Empty())
        {
            begin = this->groups_.erase(begin);   
        }
        else
        {
            ++begin;
        }
    }
}