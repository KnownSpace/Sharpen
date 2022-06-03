#include <sharpen/ByteBuffer.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/Varint.hpp>
#include <sharpen/DataCorruptionException.hpp>

sharpen::ByteBuffer::ByteBuffer(sharpen::Size size)
    :vector_(size)
{}

sharpen::ByteBuffer::ByteBuffer(Vector vector) noexcept
    :vector_(std::move(vector))
{}

sharpen::ByteBuffer::ByteBuffer(const sharpen::Char *p,sharpen::Size size)
    :vector_()
{
    for (size_t i = 0; i < size; i++)
    {
        this->vector_.push_back(p[i]);
    }
}

void sharpen::ByteBuffer::PushBack(sharpen::Char val)
{
    this->vector_.push_back(val);
}

sharpen::Size sharpen::ByteBuffer::GetSize() const noexcept
{
    return this->vector_.size();
}

void sharpen::ByteBuffer::PopBack()
{
    this->vector_.pop_back();
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
}

void sharpen::ByteBuffer::Append(const sharpen::Char *p,sharpen::Size size)
{
    if (!size)
    {
        return;
    }
    sharpen::Size oldSize{this->GetSize()};
    this->Extend(size);
    for (sharpen::Size i = oldSize,newSize = this->GetSize(); i != newSize; ++i)
    {
        this->Get(i) = *p++;
    }
}

void sharpen::ByteBuffer::Append(const sharpen::ByteBuffer &other)
{
    this->Append(other.Data(),other.GetSize());
}

void sharpen::ByteBuffer::Erase(sharpen::Size pos)
{
    this->vector_.erase(this->vector_.begin() + pos);
}

void sharpen::ByteBuffer::Erase(sharpen::Size begin,sharpen::Size end)
{
    auto ite = this->vector_.begin();
    this->vector_.erase(ite + begin,ite + end);
}

sharpen::ByteBuffer::Iterator sharpen::ByteBuffer::Find(char e) noexcept
{
    auto begin = this->Begin();
    auto end = this->End();
    while(begin != end)
    {
        if(*begin == e)
        {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ConstIterator sharpen::ByteBuffer::Find(char e) const noexcept
{
    auto begin = this->Begin();
    auto end = this->End();
    while(begin != end)
    {
        if(*begin == e)
        {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ReverseIterator sharpen::ByteBuffer::ReverseFind(char e) noexcept
{
    auto begin = this->ReverseBegin();
    auto end = this->ReverseEnd();
    while(begin != end)
    {
        if(*begin == e)
        {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ConstReverseIterator sharpen::ByteBuffer::ReverseFind(char e) const noexcept
{
    auto begin = this->ReverseBegin();
    auto end = this->ReverseEnd();
    while(begin != end)
    {
        if(*begin == e)
        {
            return begin;
        }
        ++begin;
    }
    return begin;
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