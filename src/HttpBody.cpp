#include <sharpen/HttpBody.hpp>

#include <cstring>
#include <stdexcept>
#include <cassert>

#include <sharpen/ByteBuffer.hpp>

sharpen::HttpBody::HttpBody()
    :data_()
{}

sharpen::HttpBody::HttpBody(sharpen::Size size)
    :data_(size)
{}

sharpen::HttpBody::HttpBody(const Self &other)
    :data_(other.data_)
{}

sharpen::HttpBody::HttpBody(Self &&other) noexcept
    :data_(std::move(other.data_))
{}

sharpen::HttpBody &sharpen::HttpBody::operator=(const Self &other)
{
    sharpen::HttpBody tmp(other);
    this->Swap(tmp);
    return *this;
}


sharpen::HttpBody &sharpen::HttpBody::operator=(Self &&other) noexcept
{
    this->data_ = std::move(other.data_);
    return *this;
}

void sharpen::HttpBody::Swap(Self &other) noexcept
{
    this->data_.swap(other.data_);    
}

void sharpen::HttpBody::CopyToMem(char *buf,sharpen::Size offset) const
{
    std::memcpy(buf + offset,this->Data(),this->GetSize());
}

sharpen::Size sharpen::HttpBody::CopyTo(char *buf,sharpen::Size size) const
{
    if (this->data_.empty())
    {
        return 0;
    }
    if (this->GetSize() > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    this->CopyToMem(buf,0);
    return this->GetSize();
}

sharpen::Size sharpen::HttpBody::CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    if (this->data_.empty())
    {
        return 0;
    }
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    sharpen::Size left = buf.GetSize() - offset;
    if (this->GetSize() > left)
    {
        buf.Extend(this->GetSize() - left);
    }
    this->CopyToMem(buf.Data(),offset);
    return buf.GetSize();
}

void sharpen::HttpBody::CopyFrom(const char *buf,sharpen::Size size)
{
    if (this->GetSize() < size)
    {
        this->Realloc(size);
    }
    std::memcpy(this->Data(),buf,size);
}

void sharpen::HttpBody::CopyFrom(sharpen::ByteBuffer &buf,sharpen::Size offset,sharpen::Size size)
{
    if (buf.GetSize() < (size + offset))
    {
        throw std::length_error("buffer size is wrong");
    }
    this->CopyFrom(buf.Data() + offset,size);
}

void sharpen::HttpBody::CopyFrom(sharpen::ByteBuffer &buf)
{
    this->CopyFrom(buf,0,buf.GetSize());
}

sharpen::Size sharpen::HttpBody::GetSize() const
{
    return this->data_.size();
}

sharpen::HttpBody::Iterator sharpen::HttpBody::Begin()
{
    return this->data_.begin();
}

sharpen::HttpBody::ConstIterator sharpen::HttpBody::Begin() const
{
    return this->data_.cbegin();
}

sharpen::HttpBody::ReverseIterator sharpen::HttpBody::ReverseBegin()
{
    return this->data_.rbegin();
}

sharpen::HttpBody::ConstReverseIterator sharpen::HttpBody::ReverseBegin() const
{
    return this->data_.crbegin();
}

sharpen::HttpBody::Iterator sharpen::HttpBody::End()
{
    return this->data_.end();
}

sharpen::HttpBody::ConstIterator sharpen::HttpBody::End() const
{
    return this->data_.cend();
}

sharpen::HttpBody::ReverseIterator sharpen::HttpBody::ReverseEnd()
{
    return this->data_.rend();
}

sharpen::HttpBody::ConstReverseIterator sharpen::HttpBody::ReverseEnd() const
{
    return this->data_.crend();
}

void sharpen::HttpBody::Realloc(sharpen::Size size)
{
    this->data_.resize(size,0);
}

char sharpen::HttpBody::Get(sharpen::Size index) const
{
    return this->data_.at(index);
}

char &sharpen::HttpBody::Get(sharpen::Size index)
{
    return this->data_.at(index);
}

void sharpen::HttpBody::Push(char c)
{
    this->data_.push_back(c);
}

void sharpen::HttpBody::Append(const char *buf,sharpen::Size size)
{
    for (size_t i = 0; i < size; i++)
    {
        this->Push(buf[i]);
    }
}

void sharpen::HttpBody::Erase(sharpen::Size pos)
{
    this->data_.erase(this->Begin() + pos);
}

void sharpen::HttpBody::Erase(sharpen::Size begin,sharpen::Size end)
{
    this->data_.erase(this->Begin() + begin,this->Begin() + end);
}

void sharpen::HttpBody::Erase(ConstIterator where)
{
    this->data_.erase(where);
}

void sharpen::HttpBody::Erase(ConstIterator begin,ConstIterator end)
{
    this->data_.erase(begin,end);
}

void sharpen::HttpBody::Clear()
{
    this->data_.clear();
}

char *sharpen::HttpBody::Data()
{
    return this->data_.data();
}

const char *sharpen::HttpBody::Data() const
{
    return this->data_.data();
}