#include <sharpen/RaftElectionRecord.hpp>

sharpen::RaftElectionRecord::RaftElectionRecord() noexcept
    :term_(0)
    ,votes_(0)
{}

sharpen::RaftElectionRecord::RaftElectionRecord(std::uint64_t term,std::uint64_t votes) noexcept
    :term_(term)
    ,votes_(votes)
{}

sharpen::RaftElectionRecord::RaftElectionRecord(Self &&other) noexcept
    :term_(other.term_)
    ,votes_(other.votes_)
{
    other.term_ = 0;
    other.votes_ = 0;
}

sharpen::RaftElectionRecord &sharpen::RaftElectionRecord::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->term_ = other.term_;
        this->votes_ = other.votes_;
        other.term_ = 0;
        other.votes_ = 0;
    }
    return *this;
}