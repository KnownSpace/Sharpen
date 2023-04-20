#include <sharpen/RaftPrevoteResponse.hpp>

std::size_t sharpen::RaftPrevoteResponse::ComputeSize() const noexcept
{
    return sizeof(std::uint8_t) + sizeof(this->term_);
}

std::size_t sharpen::RaftPrevoteResponse::LoadFrom(const char *data,std::size_t size)
{
    std::uint8_t status{0};
    std::uint64_t term{0};
    if(size < sizeof(status) + sizeof(term))
    {
        throw sharpen::CorruptedDataError{"corrupted prevote response"};
    }   
    std::size_t offset{0};
    std::memcpy(&status,data,sizeof(status));
    offset += sizeof(status);
    offset += sharpen::BinarySerializator::LoadFrom(term, data + offset,size - offset);
    this->status_ = status;
    this->term_ = term;
    return offset;
}

std::size_t sharpen::RaftPrevoteResponse::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint8_t status{this->status_};
    std::memcpy(data,&status,sizeof(status));
    offset += sizeof(status);
    std::uint64_t term{this->term_};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(term, data + offset);
    return offset;
}

sharpen::RaftPrevoteResponse::RaftPrevoteResponse() noexcept
    :status_(false)
    ,term_(0)
{}

sharpen::RaftPrevoteResponse::RaftPrevoteResponse(Self &&other) noexcept
    :status_(other.status_)
    ,term_(other.term_)
{
    other.status_ = false;
    other.term_ = 0;
}

sharpen::RaftPrevoteResponse &sharpen::RaftPrevoteResponse::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->status_ = other.status_;
        this->term_ = other.term_;
        other.status_ = false;
        other.term_ = 0;
    }
    return *this;
}