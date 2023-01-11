#include <sharpen/RaftVoteForRequest.hpp>

#include <sharpen/Varint.hpp>

sharpen::RaftVoteForRequest::RaftVoteForRequest(std::uint64_t id,std::uint64_t term,std::uint64_t lastIndex,std::uint64_t lastTerm) noexcept
    :id_(id)
    ,term_(term)
    ,lastIndex_(lastIndex)
    ,lastTerm_(lastTerm)
{}

sharpen::RaftVoteForRequest::RaftVoteForRequest(Self &&other) noexcept
    :id_(other.id_)
    ,term_(other.term_)
    ,lastIndex_(other.lastIndex_)
    ,lastTerm_(other.lastTerm_)
{
    other.id_ = 0;
    other.term_ = 0;
    other.lastIndex_ = 0;
    other.lastTerm_ = 0;
}

sharpen::RaftVoteForRequest &sharpen::RaftVoteForRequest::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->id_ = other.id_;
        this->term_ = other.term_;
        this->lastIndex_ = other.lastIndex_;
        this->lastTerm_ = other.lastTerm_;
        other.id_ = 0;
        other.term_ = 0;
        other.lastIndex_ = 0;
        other.lastTerm_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftVoteForRequest::ComputeSize() const noexcept
{
    sharpen::Varuint64 builder{this->GetId()};
    std::size_t size{builder.ComputeSize()};
    builder.Set(this->GetTerm());
    size += builder.ComputeSize();
    builder.Set(this->GetLastIndex());
    size += builder.ComputeSize();
    builder.Set(this->GetLastTerm());
    size += builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftVoteForRequest::LoadFrom(const char *data,std::size_t size)
{
    if(size < 4)
    {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    std::size_t offset{0};
    sharpen::Varuint64 builder{0};
    offset += builder.LoadFrom(data,size);
    this->id_ = builder.Get();
    if(size < 3 + offset)
    {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset,size - offset);
    this->term_ = builder.Get();
    if(size < 2 + offset)
    {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset,size - offset);
    this->lastIndex_ = builder.Get();
    if(size <= offset)
    {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset,size - offset);
    this->lastTerm_ = builder.Get();
    return offset;
}

std::size_t sharpen::RaftVoteForRequest::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->GetId()};
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->GetTerm());
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->GetLastIndex());
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->GetLastTerm());
    offset += builder.UnsafeStoreTo(data + offset);
    return offset;
}