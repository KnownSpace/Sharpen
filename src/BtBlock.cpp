#include <sharpen/BtBlock.hpp>

#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/IteratorOps.hpp>

sharpen::BtBlock::BtBlock()
    :BtBlock(0)
{}

sharpen::BtBlock::BtBlock(sharpen::Size blockSize)
    :BtBlock(blockSize,Self::defaultMaxRecordCount_)
{}

sharpen::BtBlock::BtBlock(sharpen::Size blockSize,sharpen::Size maxRecordCount)
    :depth_(0)
    ,next_()
    ,prev_()
    ,pairs_()
    ,blockSize_(blockSize)
    ,usedSize_(1 + sizeof(this->next_) + sizeof(this->prev_) + Self::GetCounterSize())
{
    std::memset(&this->next_,0,sizeof(this->next_));
    std::memset(&this->prev_,0,sizeof(this->prev_));
    this->pairs_.reserve(maxRecordCount);
}

sharpen::Size sharpen::BtBlock::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 2*sizeof(sharpen::FilePointer) + Self::GetCounterSize() + 1)
    {
        throw std::invalid_argument("invalid buffer");
    }
    //load depth
    sharpen::Varuint64 builder{data,size};
    sharpen::Size offset{0};
    offset += builder.ComputeSize();
    if (size < 2*sizeof(sharpen::FilePointer) + Self::GetCounterSize() + offset)
    {
        throw std::invalid_argument("invalid buffer");
    }
    this->depth_ = builder.Get();
    //load next pointer
    std::memcpy(&this->next_,data + offset,sizeof(this->next_));
    offset += sizeof(this->next_);
    //load prev pointer
    std::memcpy(&this->prev_,data + offset,sizeof(this->prev_));
    offset += sizeof(this->prev_);
    //load records
    sharpen::Size count{0};
    std::memcpy(&count,data + offset,Self::GetCounterSize());
    offset += Self::GetCounterSize();
    for (sharpen::Size i = 0; i < count; ++i)
    {
        if(offset >= size)
        {
            this->depth_ = 0;
            this->next_.offset_ = 0;
            this->next_.size_ = 0;
            this->pairs_.clear();
            throw std::invalid_argument("invalid buffer");
        }
        sharpen::BtKeyValuePair pair;
        offset += pair.LoadFrom(data + offset,size - offset);
        this->pairs_.emplace_back(std::move(pair));   
    }
    //set append pointer
    this->usedSize_ = offset;
    return offset;
}

sharpen::Size sharpen::BtBlock::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::BtBlock::UnsafeStoreTo(char *data) const
{
    //store depth
    sharpen::Varuint64 builder{this->depth_};
    sharpen::Size offset{0};
    sharpen::Size size{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),size);
    offset += size;
    //store next
    std::memcpy(data + offset,&this->next_,sizeof(this->next_));
    offset += sizeof(this->next_);
    //store prev
    std::memcpy(data + offset,&this->prev_,sizeof(this->prev_));
    offset += sizeof(this->prev_);
    //store counter
    sharpen::Uint16 count{sharpen::IntCast<sharpen::Uint16>(this->pairs_.size())};
    std::memcpy(data + offset,&count,Self::GetCounterSize());
    offset += Self::GetCounterSize();
    //store records
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        offset += begin->UnsafeStoreTo(data + offset);
    }
    return offset;
}

sharpen::Size sharpen::BtBlock::StoreTo(char *data,sharpen::Size size) const
{
    if (size < this->usedSize_)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::BtBlock::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size size{buf.GetSize() - offset};
    if(size < this->usedSize_)
    {
        buf.Extend(this->usedSize_ - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

bool sharpen::BtBlock::Comp(const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept
{
    return pair.GetKey() < key;
}

sharpen::BtBlock::Iterator sharpen::BtBlock::BinaryFind(const sharpen::ByteBuffer &key) noexcept
{
    using FnPtr = bool(*)(const sharpen::BtKeyValuePair &,const sharpen::ByteBuffer &);
    return std::lower_bound(this->Begin(),this->End(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::BtBlock::ConstIterator sharpen::BtBlock::BinaryFind(const sharpen::ByteBuffer &key) const noexcept
{
    using FnPtr = bool(*)(const sharpen::BtKeyValuePair &,const sharpen::ByteBuffer &);
    return std::lower_bound(this->Begin(),this->End(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::BtBlock::Iterator sharpen::BtBlock::FuzzingFind(const sharpen::ByteBuffer &key) noexcept
{
    auto ite = this->BinaryFind(key);
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    if (ite->GetKey() > key && this->Begin() != ite)
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::BtBlock::ConstIterator sharpen::BtBlock::FuzzingFind(const sharpen::ByteBuffer &key) const noexcept
{
    auto ite = this->BinaryFind(key);
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    if (ite->GetKey() > key && this->Begin() != ite)
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::BtBlock::Iterator sharpen::BtBlock::Find(const sharpen::ByteBuffer &key) noexcept
{
    auto ite = this->BinaryFind(key);
    if (!this->depth_)
    {
        return ite;
    }
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    if (ite->GetKey() > key && this->Begin() != ite)
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::BtBlock::ConstIterator sharpen::BtBlock::Find(const sharpen::ByteBuffer &key) const noexcept
{
    auto ite = this->BinaryFind(key);
    if (!this->depth_)
    {
        return ite;
    }
    if (ite == this->End() && !this->Empty())
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    if (ite->GetKey() > key && this->Begin() != ite)
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

void sharpen::BtBlock::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    auto ite = this->BinaryFind(key);
    sharpen::BtKeyValuePair pair{std::move(key),std::move(value)};
    if(ite != this->End())
    {
        if (ite->GetKey() == pair.GetKey())
        {
            sharpen::Size size{this->usedSize_};
            size -= ite->ComputeSize();
            size += pair.ComputeSize();
            ite->Value() = std::move(pair.Value());
            this->usedSize_ = size;
            return;
        }
        this->usedSize_ += pair.ComputeSize();
        this->pairs_.emplace(ite,std::move(pair));
        return;
    }
    this->usedSize_ += pair.ComputeSize();
    this->pairs_.emplace_back(std::move(pair));
    return;
}

void sharpen::BtBlock::Delete(const sharpen::ByteBuffer &key)
{
    auto ite = this->BinaryFind(key);
    if(ite != this->End() && ite->GetKey() == key)
    {
        sharpen::Size size{ite->ComputeSize()};
        this->pairs_.erase(ite);
        this->usedSize_ -= size;
    }
}

void sharpen::BtBlock::FuzzingDelete(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        sharpen::Size size{ite->ComputeSize()};
        this->pairs_.erase(ite);
        this->usedSize_ -= size;
    }
}

sharpen::ByteBuffer &sharpen::BtBlock::Get(const sharpen::ByteBuffer &key)
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || ite->GetKey() != key)
    {
        throw std::out_of_range("key doesn't exist");
    }
    return ite->Value();
}

const sharpen::ByteBuffer &sharpen::BtBlock::Get(const sharpen::ByteBuffer &key) const
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || ite->GetKey() != key)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return ite->Value();
}

sharpen::ExistStatus sharpen::BtBlock::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || ite->GetKey() != key)
    {
        return sharpen::ExistStatus::NotExist;
    }
    return sharpen::ExistStatus::Exist;
}

sharpen::BtBlock::PutTage sharpen::BtBlock::QueryPutTage(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->Empty() || this->ReverseBegin()->GetKey() < key)
    {
        return sharpen::BtBlock::PutTage::Append;
    }
    auto ite = this->BinaryFind(key);
    if(ite != this->End() && ite->GetKey() == this->ReverseBegin()->GetKey())
    {
        return sharpen::BtBlock::PutTage::MotifyEnd;
    }
    return sharpen::BtBlock::PutTage::Normal;
}

sharpen::BtBlock sharpen::BtBlock::Split()
{
    sharpen::BtBlock block{0};
    block.depth_ = this->depth_;
    if(!this->IsAtomic())
    {
        sharpen::Size size{this->GetSize()/2};
        auto begin = sharpen::IteratorForward(this->Begin(),this->GetSize() - size);
        auto end = this->End();
        auto ite = begin;
        block.pairs_.reserve(size);
        while (ite != end)
        {
            block.pairs_.emplace_back(std::move(*ite));
            block.usedSize_ += begin->ComputeSize();
            ++ite;
        }
        this->pairs_.erase(begin,end);
    }
    return block;
}

void sharpen::BtBlock::Combine(Self other)
{
    auto begin = other.Begin();
    auto end = other.End();
    this->pairs_.reserve(sharpen::GetRangeSize(begin,end));
    while (begin != end)
    {
        this->Put(std::move(*begin).MoveKey(),std::move(begin)->Value());
        ++begin;   
    }
}

sharpen::Size sharpen::BtBlock::ComputeCounterPointer() const noexcept
{
    // if(this->depth_ < 128)
    // {
    //     return 1 + sizeof(this->next_) + sizeof(this->prev_);
    // }
    sharpen::Varuint64 builder{this->depth_};
    return  builder.ComputeSize() + sizeof(this->prev_) + sizeof(this->next_);
}

sharpen::Size sharpen::BtBlock::ComputeNextPointer() const noexcept
{
    if(this->depth_ < 128)
    {
        return 1;
    }
    sharpen::Varuint64 builder{this->depth_};
    return builder.ComputeSize();
}

sharpen::Size sharpen::BtBlock::ComputePrevPointer() const noexcept
{
    if(this->depth_ < 128)
    {
        return 1 + sizeof(this->next_);
    }
    sharpen::Varuint64 builder{this->depth_};
    return builder.ComputeSize() + sizeof(this->next_);
}