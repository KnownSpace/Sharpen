#include <sharpen/RaftVoteForResponse.hpp>

#include <sharpen/Varint.hpp>

sharpen::RaftVoteForResponse::RaftVoteForResponse(bool status,std::uint64_t term) noexcept
    :status_(status)
    ,term_(term)
{}

sharpen::RaftVoteForResponse::RaftVoteForResponse(Self &&other) noexcept
    :status_(other.status_)
    ,term_(other.term_)
{
    other.status_ = false;
    other.term_ = 0;
}

sharpen::RaftVoteForResponse &sharpen::RaftVoteForResponse::operator=(Self &&other) noexcept
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

std::size_t sharpen::RaftVoteForResponse::ComputeSize() const noexcept
{
    std::size_t size{sizeof(std::uint8_t)};
    sharpen::Varuint64 builder{this->GetTerm()};
    size += builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftVoteForResponse::LoadFrom(const char *data,std::size_t size)
{
    if(size < 2)
    {
        throw sharpen::CorruptedDataError{"corrupted vote response"};
    }
    std::size_t offset{0};
    std::uint8_t status{0};
    std::memcpy(&status,data,sizeof(status));
    offset += sizeof(status);
    this->SetStatus(status);
    if(size < offset + 1)
    {
        throw sharpen::CorruptedDataError{"corrupted vote response"};
    }
    sharpen::Varuint64 builder{0};
    offset += builder.LoadFrom(data + offset,size - offset);
    this->term_ = builder.Get();
    return offset;
}

std::size_t sharpen::RaftVoteForResponse::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint8_t status{this->GetStatus()};
    std::memcpy(data,&status,sizeof(status));
    offset += sizeof(status);
    sharpen::Varuint64 builder{this->GetTerm()};
    offset += builder.UnsafeStoreTo(data + offset);
    return offset;
}