#include <sharpen/LogWriteResult.hpp>

#include <cassert>

sharpen::LogWriteResult::LogWriteResult(std::uint64_t lastIndex) noexcept
    :lastIndex_(lastIndex)
    ,beginIndex_(0)
{}

sharpen::LogWriteResult::LogWriteResult(std::uint64_t lastIndex,std::uint64_t beginIndex) noexcept
    :lastIndex_(lastIndex)
    ,beginIndex_(beginIndex)
{
    assert(this->lastIndex_ >= this->beginIndex_);
}

sharpen::LogWriteResult::LogWriteResult(Self &&other) noexcept
    :lastIndex_(other.lastIndex_)
    ,beginIndex_(other.beginIndex_)
{
    other.lastIndex_ = 0;
    other.beginIndex_ = 0;
}

sharpen::LogWriteResult &sharpen::LogWriteResult::operator=(const Self &other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lastIndex_ = other.lastIndex_;
        this->beginIndex_ = other.beginIndex_;
    }
    return *this;
}

sharpen::LogWriteResult &sharpen::LogWriteResult::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lastIndex_ = other.lastIndex_;
        this->beginIndex_ = other.beginIndex_;
        other.lastIndex_ = 0;
        other.beginIndex_ = 0;
    }
    return *this;
}

bool sharpen::LogWriteResult::GetStatus() const noexcept
{
    return this->beginIndex_;
}

std::uint64_t sharpen::LogWriteResult::GetLastIndex() const noexcept
{
    return this->lastIndex_;
}

sharpen::Optional<std::uint64_t> sharpen::LogWriteResult::LookupBeginIndex() const noexcept
{
    if(this->beginIndex_)
    {
        return this->beginIndex_;
    }
    return sharpen::EmptyOpt;
}

std::size_t sharpen::LogWriteResult::GetWrittenSize() const noexcept
{
    if(this->beginIndex_)
    {
        return this->lastIndex_ - this->beginIndex_ + 1;
    }
    return 0;
}