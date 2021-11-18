#include <sharpen/ByteBuffer.hpp>

#include <cstring>

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

sharpen::Size sharpen::ByteBuffer::GetSize() const
{
    return this->vector_.size();
}

void sharpen::ByteBuffer::CheckAndMoveMark()
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

const sharpen::Char *sharpen::ByteBuffer::Data() const
{
    return this->vector_.data();
}

sharpen::Char *sharpen::ByteBuffer::Data()
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

void sharpen::ByteBuffer::Reset()
{
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

sharpen::Size sharpen::ByteBuffer::Remaining() const
{
    return GetSize() - mark_;
}

sharpen::Size sharpen::ByteBuffer::GetMark() const
{
    return mark_;
}

sharpen::ByteBuffer::Iterator sharpen::ByteBuffer::Find(char e)
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

sharpen::ByteBuffer::ConstIterator sharpen::ByteBuffer::Find(char e) const
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

sharpen::ByteBuffer::ReverseIterator sharpen::ByteBuffer::ReverseFind(char e)
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

sharpen::ByteBuffer::ConstReverseIterator sharpen::ByteBuffer::ReverseFind(char e) const
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