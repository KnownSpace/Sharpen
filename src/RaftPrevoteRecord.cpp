#include <sharpen/RaftPrevoteRecord.hpp>

sharpen::RaftPrevoteRecord::RaftPrevoteRecord(Self &&other) noexcept
    : term_(other.term_)
    , votes_(std::move(other.votes_)) {
    other.term_ = 0;
}

sharpen::RaftPrevoteRecord &sharpen::RaftPrevoteRecord::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->term_ = other.term_;
        this->votes_ = std::move(other.votes_);
        other.term_ = 0;
    }
    return *this;
}

void sharpen::RaftPrevoteRecord::Flush() noexcept {
    this->votes_.clear();
}

void sharpen::RaftPrevoteRecord::Flush(std::uint64_t term) noexcept {
    this->term_ = term;
    this->votes_.clear();
}

std::uint64_t sharpen::RaftPrevoteRecord::GetVotes() const noexcept {
    return this->votes_.size();
}

void sharpen::RaftPrevoteRecord::Receive(const sharpen::ActorId &actorId) {
    this->votes_.emplace(actorId);
}