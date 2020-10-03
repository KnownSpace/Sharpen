#include <sharpen/ByteBuffer.hpp>

void sharpen::ByteBuffer::swap(sharpen::ByteBuffer &other) noexcept
{
    vector_.swap(other.vector_);
    mark_ = other.mark_;
}

sharpen::ByteBuffer::ByteBuffer(sharpen::Size size)
    :vector_(size)
    ,mark_(0)
{}

sharpen::ByteBuffer::ByteBuffer(Vector &&vector) noexcept
    :vector_(std::move(vector))
    ,mark_(0)
{}

sharpen::ByteBuffer::ByteBuffer(const sharpen::Char *p,sharpen::Size size)
    :vector_(size)
    ,mark_(0)
{
    for (size_t i = 0; i < size; i++)
    {
        this->vector_.push_back(p[i]);
    }
}

sharpen::ByteBuffer::ByteBuffer(sharpen::ByteBuffer &&other) noexcept
    :vector_(std::move(other.vector_))
    ,mark_(other.mark_)
{}

sharpen::ByteBuffer &sharpen::ByteBuffer::operator=(sharpen::ByteBuffer &&other) noexcept
{
    this->vector_ = std::move(other.vector_);
    this->mark_ = other.mark_;
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
    return reinterpret_cast<sharpen::Char*>(this->Data());
}

void sharpen::ByteBuffer::Reserve(sharpen::Size size)
{
    this->vector_.reserve(size);
}

void sharpen::ByteBuffer::Expand(sharpen::Size size)
{
    this->Reserve(size + this->GetSize());
}

void sharpen::ByteBuffer::Shrink()
{
    this->vector_.shrink_to_fit();
}

void sharpen::ByteBuffer::Append(const sharpen::Char *p,sharpen::Size size)
{
    if (size == 0)
    {
        return;
    }
    this->Expand(size);
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