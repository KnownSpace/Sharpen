#include <sharpen/RaftPrevoteRequest.hpp>

std::size_t sharpen::RaftPrevoteRequest::ComputeSize() const noexcept
{
    return sizeof(this->lastIndex_) + sizeof(this->lastTerm_);
}

std::size_t sharpen::RaftPrevoteRequest::LoadFrom(const char *data, std::size_t size)
{
    if (size < sizeof(this->lastIndex_) + sizeof(this->lastTerm_))
    {
        throw sharpen::CorruptedDataError{"corrupted prevote request"};
    }
    std::uint64_t lastIndex{0};
    std::uint64_t lastTerm{0};
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::LoadFrom(lastIndex, data, size);
    offset += sharpen::BinarySerializator::LoadFrom(lastTerm, data + offset, size - offset);
    offset += sizeof(this->lastTerm_);
    this->lastIndex_ = lastIndex;
    this->lastTerm_ = lastTerm;
    return offset;
}

std::size_t sharpen::RaftPrevoteRequest::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint64_t lastIndex{this->lastIndex_};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(lastIndex, data);
    std::uint64_t lastTerm{this->lastTerm_};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(lastTerm, data + offset);
    return offset;
}

sharpen::RaftPrevoteRequest::RaftPrevoteRequest() noexcept
    : lastIndex_(0)
    , lastTerm_(0)
{
}

sharpen::RaftPrevoteRequest::RaftPrevoteRequest(Self &&other) noexcept
    : lastIndex_(other.lastIndex_)
    , lastTerm_(other.lastTerm_)
{
    other.lastIndex_ = 0;
    other.lastTerm_ = 0;
}

sharpen::RaftPrevoteRequest &sharpen::RaftPrevoteRequest::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->lastIndex_ = other.lastIndex_;
        this->lastTerm_ = other.lastTerm_;
        other.lastIndex_ = 0;
        other.lastTerm_ = 0;
    }
    return *this;
}