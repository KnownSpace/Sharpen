#include <sharpen/ByteBuffer.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/DataCorruptionException.hpp>

void sharpen::ByteBuffer::swap(sharpen::ByteBuffer &other) noexcept
{
    vector_.swap(other.vector_);
    mark_ = other.mark_;
}

sharpen::ByteBuffer::ByteBuffer()
    :vector_()
    ,mark_(0)
{}

sharpen::ByteBuffer::ByteBuffer(sharpen::Size size)
    :vector_(size)
    ,mark_(0)
{}

sharpen::ByteBuffer::ByteBuffer(Vector &&vector) noexcept
    :vector_(std::move(vector))
    ,mark_(0)
{}

sharpen::ByteBuffer::ByteBuffer(const sharpen::Char *p,sharpen::Size size)
    :vector_()
    ,mark_(0)
{
    for (size_t i = 0; i < size; i++)
    {
        this->vector_.push_back(p[i]);
    }
}

sharpen::ByteBuffer::ByteBuffer(const sharpen::ByteBuffer &other)
    :vector_(other.vector_)
    ,mark_(other.mark_)
{}

sharpen::ByteBuffer::ByteBuffer(sharpen::ByteBuffer &&other) noexcept
    :vector_(std::move(other.vector_))
    ,mark_(other.mark_)
{
    other.mark_ = 0;
}

sharpen::ByteBuffer &sharpen::ByteBuffer::operator=(const sharpen::ByteBuffer &other)
{
    if(this != std::addressof(other))
    {
        sharpen::ByteBuffer tmp{other};
        std::swap(tmp,*this);
    }
    return *this;
}

sharpen::ByteBuffer &sharpen::ByteBuffer::operator=(sharpen::ByteBuffer &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->vector_ = std::move(other.vector_);
        this->mark_ = other.mark_;
        other.mark_ = 0;
    }
    return *this;
}

void sharpen::ByteBuffer::PushBack(sharpen::Char val)
{
    this->vector_.push_back(val);
}

sharpen::Size sharpen::ByteBuffer::GetSize() const noexcept
{
    return this->vector_.size();
}

void sharpen::ByteBuffer::CheckAndMoveMark() noexcept
{
    if (this->mark_ > this->vector_.size())
    {
        this->mark_ = this->vector_.size();
    }
}

void sharpen::ByteBuffer::PopBack()
{
    this->vector_.pop_back();
    this->CheckAndMoveMark();
}

sharpen::Char sharpen::ByteBuffer::Back() const
{
    return this->vector_.back();
}

sharpen::Char &sharpen::ByteBuffer::Back()
{
    return this->vector_.back();
}

sharpen::Char sharpen::ByteBuffer::Front() const
{
    return this->vector_.front();
}

sharpen::Char &sharpen::ByteBuffer::Front()
{
    return this->vector_.front();
}

sharpen::Char sharpen::ByteBuffer::Get(sharpen::Size index) const
{
    return this->vector_.at(index);
}

sharpen::Char &sharpen::ByteBuffer::Get(sharpen::Size index)
{
    return this->vector_.at(index);
}

const sharpen::Char *sharpen::ByteBuffer::Data() const noexcept
{
    return this->vector_.data();
}

sharpen::Char *sharpen::ByteBuffer::Data() noexcept
{
    return reinterpret_cast<sharpen::Char*>(this->vector_.data());
}

void sharpen::ByteBuffer::Reserve(sharpen::Size size)
{
    this->vector_.reserve(size);
}

void sharpen::ByteBuffer::Extend(sharpen::Size size,sharpen::Char defaultValue)
{
    this->vector_.resize(this->GetSize() + size,defaultValue);
}

void sharpen::ByteBuffer::Extend(sharpen::Size size)
{
    this->Extend(size,0);
}

void sharpen::ByteBuffer::ExtendTo(sharpen::Size size,sharpen::Char defaultValue)
{
    this->vector_.resize(size,defaultValue);
}

void sharpen::ByteBuffer::ExtendTo(sharpen::Size size)
{
    this->ExtendTo(size,0);
}

void sharpen::ByteBuffer::Reset() noexcept
{
    if(this->Empty())
    {
        return;
    }
    std::memset(this->vector_.data(),0,this->vector_.size());
}

void sharpen::ByteBuffer::Shrink()
{
    this->vector_.shrink_to_fit();
    this->CheckAndMoveMark();
}

void sharpen::ByteBuffer::Append(const sharpen::Char *p,sharpen::Size size)
{
    if (size == 0)
    {
        return;
    }
    for (size_t i = 0; i < size; i++)
    {
        this->PushBack(p[i]);
    }
}

void sharpen::ByteBuffer::Append(const sharpen::ByteBuffer &other)
{
    this->Append(other.Data(),other.GetSize());
}

void sharpen::ByteBuffer::Erase(sharpen::Size pos)
{
    this->vector_.erase(this->vector_.begin() + pos);
    this->CheckAndMoveMark();
}

void sharpen::ByteBuffer::Erase(sharpen::Size begin,sharpen::Size end)
{
    auto ite = this->vector_.begin();
    this->vector_.erase(ite + begin,ite + end);
    this->CheckAndMoveMark();
}

void sharpen::ByteBuffer::Mark(sharpen::Size pos)
{
    mark_ = pos;
    this->CheckAndMoveMark();
}

sharpen::Size sharpen::ByteBuffer::Remaining() const noexcept
{
    return GetSize() - mark_;
}

sharpen::Size sharpen::ByteBuffer::GetMark() const noexcept
{
    return mark_;
}

sharpen::ByteBuffer::Iterator sharpen::ByteBuffer::Find(char e) noexcept
{
    for (auto begin = this->Begin(); begin != this->End(); begin++)
    {
        if (*begin == e)
        {
            return begin;
        }
    }
    return this->End();
}

sharpen::ByteBuffer::ConstIterator sharpen::ByteBuffer::Find(char e) const noexcept
{
    for (auto begin = this->Begin(); begin != this->End(); ++begin)
    {
        if (*begin == e)
        {
            return begin;
        }
    }
    return this->End();
}

sharpen::ByteBuffer::ReverseIterator sharpen::ByteBuffer::ReverseFind(char e) noexcept
{
    for (auto begin = this->ReverseBegin(); begin != this->ReverseEnd(); ++begin)
    {
        if (*begin == e)
        {
            return begin;
        }
    }
    return this->ReverseEnd();
}

sharpen::ByteBuffer::ConstReverseIterator sharpen::ByteBuffer::ReverseFind(char e) const noexcept
{
    for (auto begin = this->ReverseBegin(); begin != this->ReverseEnd(); begin++)
    {
        if (*begin == e)
        {
            return begin;
        }
    }
    return this->ReverseEnd();
}

void sharpen::ByteBuffer::Erase(ConstIterator where)
{
    this->vector_.erase(where);
}

void sharpen::ByteBuffer::Erase(ConstIterator begin,ConstIterator end)
{
    this->vector_.erase(begin,end);
}

sharpen::Char sharpen::ByteBuffer::GetOrDefault(sharpen::Size index,sharpen::Char defaultVal) const noexcept
{
    if(index >= this->GetSize())
    {
        return defaultVal;
    }
    return this->Get(index);
}

sharpen::Size sharpen::ByteBuffer::ComputeSize() const noexcept
{
    sharpen::Size offset{0};
    sharpen::Varuint64 builder{this->GetSize()};
    offset += builder.ComputeSize();
    offset += this->GetSize();
    return offset;
}

sharpen::Size sharpen::ByteBuffer::LoadFrom(const char *data,sharpen::Size size)
{
    sharpen::Size offset{0};
    sharpen::Varuint64 builder{data,size};
    sharpen::Size sz{builder.ComputeSize()};
    offset += sz;
    sz = builder.Get();
    if(sz)
    {
        if (size < offset + sz)
        {
            this->Clear();
            throw sharpen::DataCorruptionException("byte buffer corruption");
        }
        this->ExtendTo(sz);
        std::memcpy(this->Data(),data + offset,sz);
        offset += sz;
    }
    return offset;
}

sharpen::Size sharpen::ByteBuffer::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::ByteBuffer::UnsafeStoreTo(char *data) const noexcept
{
    sharpen::Size offset{0};
    sharpen::Varuint64 builder{this->GetSize()};
    sharpen::Size size{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),size);
    offset += size;
    std::memcpy(data + offset,this->Data(),this->GetSize());
    offset += this->GetSize();
    return offset;
}

sharpen::Size sharpen::ByteBuffer::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(needSize > size)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::ByteBuffer::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size size{buf.GetSize() - offset};
    sharpen::Size needSize{this->ComputeSize()};
    if(needSize > size)
    {
        buf.ExtendTo(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}