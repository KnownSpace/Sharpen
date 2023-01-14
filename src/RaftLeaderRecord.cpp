#include <sharpen/RaftLeaderRecord.hpp>

sharpen::RaftLeaderRecord::RaftLeaderRecord() noexcept
    :term_(0)
    ,leaderId_(0)
{}

sharpen::RaftLeaderRecord::RaftLeaderRecord(const Self &other) noexcept
    :term_(other.term_.load())
    ,leaderId_(other.leaderId_)
{}

sharpen::RaftLeaderRecord::RaftLeaderRecord(std::uint64_t term,std::uint64_t leaderId) noexcept
    :term_(term)
    ,leaderId_(leaderId)
{}

sharpen::RaftLeaderRecord::RaftLeaderRecord(Self &&other) noexcept
    :term_(other.term_.load())
    ,leaderId_(other.leaderId_)
{
    other.term_ = 0;
    other.leaderId_ = 0;
}

sharpen::RaftLeaderRecord &sharpen::RaftLeaderRecord::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->term_ = other.term_.load();
        this->leaderId_ = other.leaderId_;
        other.term_ = 0;
        other.leaderId_ = 0;
    }
    return *this;
}

sharpen::Optional<std::pair<std::uint64_t,std::uint64_t>> sharpen::RaftLeaderRecord::GetRecord() const noexcept
{
    std::uint64_t term{this->term_.load(std::memory_order::memory_order_acquire)};
    if(!term)
    {
        return sharpen::EmptyOpt;
    }
    return {term,this->leaderId_};
}

void sharpen::RaftLeaderRecord::Flush(std::uint64_t term,std::uint64_t leaderId) noexcept
{
    //set term to be zero
    //then other threads could not acquire
    //outdated leader id
    this->term_.store(0,std::memory_order::memory_order_seq_cst);
    //set leader id
    this->leaderId_ = leaderId;
    //release leader id
    //now it could be acquired by other threads
    this->term_.store(term,std::memory_order::memory_order_release);
}