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
    const char *lhs{this->Data()};
    const char *rhs{other.Data()};
    std::size_t leftSz{this->GetSize()};
    std::size_t rightSz{other.GetSize()};
    std::int32_t r{0};
    if(lhs && rhs)
    {
        r = std::memcmp(lhs,rhs,(std::min)(leftSz,rightSz));
    }
    if(r)
    {
        return r > 0 ? 1:-1;
    }
    if(leftSz < rightSz)
    {
        return -1;
    }
    else if(leftSz > rightSz)
    {
        return 1;
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