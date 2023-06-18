#include <sharpen/RaftElectionRecord.hpp>

#include <sharpen/ConsensusWriter.hpp>

sharpen::RaftElectionRecord::RaftElectionRecord() noexcept
    : term_(sharpen::ConsensusWriter::noneEpoch)
    , votes_(0) {
}

sharpen::RaftElectionRecord::RaftElectionRecord(std::uint64_t term, std::uint64_t votes) noexcept
    : term_(term)
    , votes_(votes) {
}

sharpen::RaftElectionRecord::RaftElectionRecord(Self &&other) noexcept
    : term_(other.term_)
    , votes_(other.votes_) {
    other.term_ = sharpen::ConsensusWriter::noneEpoch;
    other.votes_ = 0;
}

sharpen::RaftElectionRecord &sharpen::RaftElectionRecord::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->term_ = other.term_;
        this->votes_ = other.votes_;
        other.term_ = sharpen::ConsensusWriter::noneEpoch;
        other.votes_ = 0;
    }
    return *this;
}