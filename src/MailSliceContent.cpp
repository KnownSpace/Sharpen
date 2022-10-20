#include <sharpen/MailSliceContent.hpp>

#include <cassert>

sharpen::MailSliceContent::MailSliceContent(char *slice,std::size_t size) noexcept
    :slice_(slice)
    ,sliceSize_(size)
{
    assert(this->slice_ && this->sliceSize_);
}

sharpen::MailSliceContent::MailSliceContent(Self &&other) noexcept
    :slice_(other.slice_)
    ,sliceSize_(other.sliceSize_)
{
    assert(this->slice_ && this->sliceSize_);
    other.slice_ = nullptr;
    other.sliceSize_ = 0;
}

sharpen::MailSliceContent &sharpen::MailSliceContent::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->slice_ = other.slice_;
        this->sliceSize_ = other.sliceSize_;
        other.slice_ = nullptr;
        other.sliceSize_ = 0;
        assert(this->slice_ && this->sliceSize_);
    }
    return *this;
}

char *sharpen::MailSliceContent::DoContent() noexcept
{
    return this->slice_;
}

const char *sharpen::MailSliceContent::DoContent() const noexcept
{
    return this->slice_;
}

std::size_t sharpen::MailSliceContent::DoGetSize() const noexcept
{
    return this->sliceSize_;
}

void sharpen::MailSliceContent::DoResize(std::size_t newSize) 
{
    (void)newSize;
    throw sharpen::SliceResizeError{};
}