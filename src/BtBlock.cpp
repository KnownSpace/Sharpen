#include <sharpen/BtBlock.hpp>

#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/IteratorOps.hpp>

sharpen::BtBlock::BtBlock()
    :BtBlock(0)
{}

sharpen::BtBlock::BtBlock(std::size_t blockSize)
    :BtBlock(blockSize,Self::defaultMaxRecordCount_)
{}

sharpen::BtBlock::BtBlock(std::size_t blockSize,std::size_t maxRecordCount)
    :depth_(0)
    ,next_()
    ,prev_()
    ,pairs_()
    ,blockSize_(blockSize)
    ,usedSize_(1 + sizeof(this->next_) + sizeof(this->prev_) + Self::GetCounterSize())
    ,switzzPointer_(0)
    ,comp_(nullptr)
{
    std::memset(&this->next_,0,sizeof(this->next_));
    std::memset(&this->prev_,0,sizeof(this->prev_));
    this->pairs_.reserve(maxRecordCount);
}

std::size_t sharpen::BtBlock::LoadFrom(const char *data,std::size_t size)
{
    if(size < 2*sizeof(sharpen::FilePointer) + Self::GetCounterSize() + 1)
    {
        throw std::invalid_argument("invalid buffer");
    }
    //load depth
    sharpen::Varuint64 builder{data,size};
    std::size_t offset{0};
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
    std::size_t count{0};
    std::memcpy(&count,data + offset,Self::GetCounterSize());
    offset += Self::GetCounterSize();
    for (std::size_t i = 0; i < count; ++i)
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

std::size_t sharpen::BtBlock::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::BtBlock::UnsafeStoreTo(char *data) const
{
    //store depth
    sharpen::Varuint64 builder{this->depth_};
    std::size_t offset{0};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),size);
    offset += size;
    //store next
    std::memcpy(data + offset,&this->next_,sizeof(this->next_));
    offset += sizeof(this->next_);
    //store prev
    std::memcpy(data + offset,&this->prev_,sizeof(this->prev_));
    offset += sizeof(this->prev_);
    //store counter
    std::uint16_t count{sharpen::IntCast<std::uint16_t>(this->pairs_.size())};
    std::memcpy(data + offset,&count,Self::GetCounterSize());
    offset += Self::GetCounterSize();
    //store records
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        offset += begin->UnsafeStoreTo(data + offset);
    }
    return offset;
}

std::size_t sharpen::BtBlock::StoreTo(char *data,std::size_t size) const
{
    if (size < this->usedSize_)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::BtBlock::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    if(size < this->usedSize_)
    {
        buf.Extend(this->usedSize_ - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

sharpen::BtBlock &sharpen::BtBlock::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->depth_ = other.depth_;
        this->next_ = std::move(other.next_);
        this->prev_ = std::move(other.prev_);
        this->pairs_ = std::move(other.pairs_);
        this->blockSize_ = other.blockSize_;
        this->usedSize_ = other.usedSize_;
        this->switzzPointer_ = other.switzzPointer_;
        this->comp_ = other.comp_;
        other.usedSize_ = 1 + sizeof(this->next_) + sizeof(this->prev_) + sizeof(std::uint16_t);
        other.depth_ = 0;
        other.switzzPointer_ = 0;
        other.blockSize_ = 0;
        other.comp_ = nullptr;
    }
    return *this;
}

bool sharpen::BtBlock::WarppedComp(Comparator comp,const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept
{
    if(!comp)
    {
        return Self::Comp(pair,key);
    }
    return comp(pair.GetKey(),key) == -1;
}

bool sharpen::BtBlock::Comp(const sharpen::BtKeyValuePair &pair,const sharpen::ByteBuffer &key) noexcept
{
    return pair.GetKey() < key;
}

std::int32_t sharpen::BtBlock::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::BtBlock::Iterator sharpen::BtBlock::BinaryFind(const sharpen::ByteBuffer &key) noexcept
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::BtKeyValuePair&,const sharpen::ByteBuffer &);
        return std::lower_bound(this->Begin(),this->End(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
    using FnPtr = bool(*)(const sharpen::BtKeyValuePair &,const sharpen::ByteBuffer &);
    return std::lower_bound(this->Begin(),this->End(),key,static_cast<FnPtr>(&Self::Comp));
}

sharpen::BtBlock::ConstIterator sharpen::BtBlock::BinaryFind(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->comp_)
    {
        using Warp = bool(*)(Comparator,const sharpen::BtKeyValuePair&,const sharpen::ByteBuffer &);
        return std::lower_bound(this->Begin(),this->End(),key,std::bind(static_cast<Warp>(&Self::WarppedComp),this->comp_,std::placeholders::_1,std::placeholders::_2));
    }
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
    if (this->CompKey(ite->GetKey(),key) == 1 && this->Begin() != ite)
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
    if (this->CompKey(ite->GetKey(),key) == 1 && this->Begin() != ite)
    {
        ite = sharpen::IteratorBackward(ite,1);
    }
    return ite;
}

sharpen::BtBlock::Iterator sharpen::BtBlock::Find(const sharpen::ByteBuffer &key) noexcept
{
    if(this->depth_)
    {
        return this->FuzzingFind(key);
    }
    return this->BinaryFind(key);
}

sharpen::BtBlock::ConstIterator sharpen::BtBlock::Find(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->depth_)
    {
        return this->FuzzingFind(key);
    }
    return this->BinaryFind(key);
}

void sharpen::BtBlock::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    auto ite = this->BinaryFind(key);
    sharpen::BtKeyValuePair pair{std::move(key),std::move(value)};
    if(ite != this->End())
    {
        if (this->CompKey(ite->GetKey(),pair.GetKey()) == 0)
        {
            std::size_t size{this->usedSize_};
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
    if(ite != this->End() && this->CompKey(ite->GetKey(),key) == 0)
    {
        std::size_t size{ite->ComputeSize()};
        this->pairs_.erase(ite);
        this->usedSize_ -= size;
    }
}

void sharpen::BtBlock::FuzzingDelete(const sharpen::ByteBuffer &key)
{
    auto ite = this->Find(key);
    if(ite != this->End())
    {
        std::size_t size{ite->ComputeSize()};
        this->pairs_.erase(ite);
        this->usedSize_ -= size;
    }
}

sharpen::ByteBuffer &sharpen::BtBlock::Get(const sharpen::ByteBuffer &key)
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || this->CompKey(ite->GetKey(),key) != 0)
    {
        throw std::out_of_range("key doesn't exist");
    }
    return ite->Value();
}

const sharpen::ByteBuffer &sharpen::BtBlock::Get(const sharpen::ByteBuffer &key) const
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || this->CompKey(ite->GetKey(),key) != 0)
    {
        throw std::out_of_range("key doesn't exists");
    }
    return ite->Value();
}

sharpen::ExistStatus sharpen::BtBlock::Exist(const sharpen::ByteBuffer &key) const
{
    auto ite = this->BinaryFind(key);
    if(ite == this->End() || this->CompKey(ite->GetKey(),key) != 0)
    {
        return sharpen::ExistStatus::NotExist;
    }
    return sharpen::ExistStatus::Exist;
}

sharpen::BtBlock::PutTage sharpen::BtBlock::QueryPutTage(const sharpen::ByteBuffer &key) const noexcept
{
    if(this->Empty() || this->CompKey(this->ReverseBegin()->GetKey(),key) == -1)
    {
        return sharpen::BtBlock::PutTage::Append;
    }
    auto ite = this->BinaryFind(key);
    if(ite != this->End() && this->CompKey(ite->GetKey(),this->ReverseBegin()->GetKey()) == 0)
    {
        return sharpen::BtBlock::PutTage::MotifyEnd;
    }
    return sharpen::BtBlock::PutTage::Normal;
}

sharpen::BtBlock sharpen::BtBlock::Split()
{
    sharpen::BtBlock block{0};
    block.depth_ = this->depth_;
    block.comp_ = this->comp_;
    if(!this->IsAtomic())
    {
        std::size_t size{this->GetSize()/2};
        auto begin = sharpen::IteratorForward(this->Begin(),this->GetSize() - size);
        auto end = this->End();
        auto ite = begin;
        block.pairs_.reserve(size);
        while (ite != end)
        {
            block.pairs_.emplace_back(std::move(*ite));
            std::size_t kvSize{begin->ComputeSize()};
            block.usedSize_ += kvSize;
            this->usedSize_ -= kvSize;
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

std::size_t sharpen::BtBlock::ComputeCounterPointer() const noexcept
{
    if(this->depth_ < 128)
    {
        return 1 + sizeof(this->next_) + sizeof(this->prev_);
    }
    sharpen::Varuint64 builder{this->depth_};
    return  builder.ComputeSize() + sizeof(this->prev_) + sizeof(this->next_);
}

std::size_t sharpen::BtBlock::ComputeNextPointer() const noexcept
{
    if(this->depth_ < 128)
    {
        return 1;
    }
    sharpen::Varuint64 builder{this->depth_};
    return builder.ComputeSize();
}

std::size_t sharpen::BtBlock::ComputePrevPointer() const noexcept
{
    if(this->depth_ < 128)
    {
        return 1 + sizeof(this->next_);
    }
    sharpen::Varuint64 builder{this->depth_};
    return builder.ComputeSize() + sizeof(this->next_);
}