#include <sharpen/MicroRpcStack.hpp>

void sharpen::MicroRpcStack::UnsafeCopyTo(char *data) const noexcept
{
    sharpen::Size size{0};
    for (auto begin = this->fields_.begin(), end = this->fields_.end(); begin != end;)
    {
        auto cbegin = begin;
        cbegin->CopyTo(++begin == end, data + size, cbegin->GetRawSize());
        size += cbegin->GetRawSize();
    }
}

void sharpen::MicroRpcStack::CopyTo(sharpen::ByteBuffer &buf, sharpen::Size offset)
{
    if (buf.GetSize() < this->ComputeSize() + offset)
    {
        buf.ExtendTo(this->ComputeSize() + offset);
    }
    this->UnsafeCopyTo(buf.Data());
}

void sharpen::MicroRpcStack::CopyTo(char *data, sharpen::Size size) const
{
    if (size < this->ComputeSize())
    {
        throw std::length_error("buf too small");
    }
    this->UnsafeCopyTo(data);
}

sharpen::Size sharpen::MicroRpcStack::ComputeSize() const noexcept
{
    sharpen::Size totalSize{0};
    for (auto begin = this->fields_.begin(), end = this->fields_.end(); begin != end; ++begin)
    {
        totalSize += begin->GetRawSize();
    }
    return totalSize;
}