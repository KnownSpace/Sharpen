#include <sharpen/RaftVoteRecord.hpp>

sharpen::RaftVoteRecord::RaftVoteRecord(std::uint64_t term,std::uint64_t actorId) noexcept
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
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::LoadFrom(this->term_,data,sizeof(this->term_));
    offset += sharpen::BinarySerializator::LoadFrom(this->actorId_,data + offset,sizeof(this->actorId_));
    return offset;
}

std::size_t sharpen::RaftVoteRecord::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->term_,data);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->actorId_,data + offset);
    return offset;
}