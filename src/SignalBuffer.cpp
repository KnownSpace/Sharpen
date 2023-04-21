#include <sharpen/SignalBuffer.hpp>

sharpen::SignalBuffer::SignalBuffer(std::size_t size)
    : buf_(size)
    , offset_(0)
{
}

sharpen::SignalBuffer::SignalBuffer(Self &&other) noexcept
    : buf_(std::move(other.buf_))
    , offset_(other.offset_)
{
    other.offset_ = 0;
}

sharpen::SignalBuffer &sharpen::SignalBuffer::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->buf_ = std::move(other.buf_);
        this->offset_ = other.offset_;
        other.offset_ = 0;
    }
    return *this;
}

std::int32_t sharpen::SignalBuffer::PopSignal() noexcept
{
    std::int32_t sig{0};
    if (this->offset_ != this->GetSize())
    {
        sig = this->buf_[this->offset_];
        this->offset_ += 1;
    }
    return sig;
}