#include <sharpen/RaftVoteRecord.hpp>

sharpen::RaftVoteRecord::RaftVoteRecord(std::uint64_t term,std::uint64_t actorId)
    :term_(term)
    ,actorId_(actorId)
{}

std::size_t sharpen::RaftVoteRecord::ComputeSize() const noexcept
{
    return sizeof(this->term_) + sizeof(this->actorId_);
}

std::size_t sharpen::RaftVoteRecord::LoadFrom(const char *data,std::size_t size)
{
    if(size < this->ComputeSize())
    {
        throw sharpen::CorruptedDataError{"corrupted raft vote record"};
    }
    std::memcpy(&this->term_,data,sizeof(this->term_));
    std::memcpy(&this->actorId_,data + sizeof(this->term_),sizeof(this->actorId_));
    return this->ComputeSize();
}

std::size_t sharpen::RaftVoteRecord::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::memcpy(data,&this->term_,sizeof(this->term_));
    offset += sizeof(this->term_);
    std::memcpy(data + offset,&this->actorId_,sizeof(this->actorId_));
    offset += sizeof(this->actorId_);
    return offset;
}