#include <sharpen/RaftLeaseStatus.hpp>

sharpen::RaftLeaseStatus::RaftLeaseStatus() noexcept
    : leaseRound_(0)
    , ackCount_(0) {
}

sharpen::RaftLeaseStatus::RaftLeaseStatus(const Self &other) noexcept
    : leaseRound_(other.leaseRound_)
    , ackCount_(other.ackCount_) {
}

sharpen::RaftLeaseStatus::RaftLeaseStatus(Self &&other) noexcept
    : leaseRound_(other.leaseRound_)
    , ackCount_(other.ackCount_) {
    other.leaseRound_ = 0;
    other.ackCount_ = 0;
}

void sharpen::RaftLeaseStatus::NextRound() noexcept {
    this->ackCount_ = 0;
    this->leaseRound_ += 1;
}

std::uint64_t sharpen::RaftLeaseStatus::GetRound() const noexcept {
    return this->leaseRound_;
}

void sharpen::RaftLeaseStatus::OnAck() noexcept {
    this->ackCount_ += 1;
}

std::size_t sharpen::RaftLeaseStatus::GetAckCount() const noexcept {
    return this->ackCount_;
}