#include <sharpen/LogEntries.hpp>

#include <cassert>

sharpen::LogEntries &sharpen::LogEntries::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->logs_ = std::move(other.logs_);
    }
    return *this;
}

std::size_t sharpen::LogEntries::GetSize() const noexcept
{
    return this->logs_.size();
}

sharpen::ByteBuffer &sharpen::LogEntries::Get(std::size_t index) noexcept
{
    assert(this->GetSize() > index);
    return this->logs_[index];
}

const sharpen::ByteBuffer &sharpen::LogEntries::Get(std::size_t index) const noexcept
{
    assert(this->GetSize() > index);
    return this->logs_[index];
}

void sharpen::LogEntries::Push(sharpen::ByteBuffer log)
{
    this->logs_.emplace_back(std::move(log));
}

void sharpen::LogEntries::Reserve(std::size_t size)
{
    std::size_t newSize{this->GetSize() + size};
    this->logs_.reserve(newSize);
}

std::size_t sharpen::LogEntries::ComputeSize() const noexcept
{
    return sharpen::BinarySerializator::ComputeSize(this->logs_);
}

std::size_t sharpen::LogEntries::LoadFrom(const char *data,std::size_t size)
{
    try
    {
        return sharpen::BinarySerializator::LoadFrom(this->logs_,data,size);
    }
    catch(const sharpen::CorruptedDataError &rethrow)
    {
        throw sharpen::CorruptedDataError{"corrupted log entries"};
        (void)rethrow;
    }
}

std::size_t sharpen::LogEntries::UnsafeStoreTo(char *data) const noexcept
{
    return sharpen::BinarySerializator::UnsafeStoreTo(this->logs_,data);
}