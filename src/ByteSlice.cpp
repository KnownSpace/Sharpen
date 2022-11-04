#include <sharpen/ByteSlice.hpp>

sharpen::ByteSlice &sharpen::ByteSlice::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->data_ = other.data_;
        this->size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

char sharpen::ByteSlice::Get(std::size_t index) const
{
    if(index > this->size_)
    {
        throw std::out_of_range{"index out of range"};
    }
    return this->data_[index];
}

std::int32_t sharpen::ByteSlice::CompareWith(const Self &other) const noexcept
{
    std::int32_t r{std::memcmp(this->data_,other.Data(),(std::min)(this->size_,other.GetSize()))};
    if(r)
    {
        return r;
    }
    if(this->size_ > other.GetSize())
    {
        return 1;
    }
    else if(this->size_ < other.GetSize())
    {
        return -1;
    }
    return 0;
}

sharpen::ByteSlice::ConstIterator sharpen::ByteSlice::Find(char e) const noexcept
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

sharpen::ByteSlice::ConstReverseIterator sharpen::ByteSlice::ReverseFind(char e) const noexcept
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