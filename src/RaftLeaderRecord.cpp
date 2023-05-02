#include <sharpen/RaftLeaderRecord.hpp>

sharpen::RaftLeaderRecord::RaftLeaderRecord() noexcept
    : term_(0)
    , leaderId_(0) {
}

sharpen::RaftLeaderRecord::RaftLeaderRecord(const Self &other) noexcept
    : term_(other.term_.load(std::memory_order::memory_order_relaxed))
    , leaderId_(other.leaderId_.load(std::memory_order::memory_order_relaxed)) {
}

sharpen::RaftLeaderRecord::RaftLeaderRecord(std::uint64_t term, std::uint64_t leaderId) noexcept
    : term_(term)
    , leaderId_(leaderId) {
}

sharpen::RaftLeaderRecord::RaftLeaderRecord(Self &&other) noexcept
    : term_(other.term_.load(std::memory_order::memory_order_relaxed))
    , leaderId_(other.leaderId_.load(std::memory_order::memory_order_relaxed)) {
    other.term_.store(0, std::memory_order::memory_order_relaxed);
    other.leaderId_.store(0, std::memory_order::memory_order_relaxed);
}

sharpen::RaftLeaderRecord &sharpen::RaftLeaderRecord::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        std::uint64_t term{other.term_.load(std::memory_order::memory_order_relaxed)};
        std::uint64_t leaderId{other.leaderId_.load(std::memory_order::memory_order_relaxed)};
        this->term_ = term;
        this->leaderId_ = leaderId;
        other.term_.store(0, std::memory_order::memory_order_relaxed);
        other.leaderId_.store(0, std::memory_order::memory_order_relaxed);
    }
    return *this;
}

std::pair<std::uint64_t, std::uint64_t> sharpen::RaftLeaderRecord::GetRecord() const noexcept {
    std::uint64_t term{this->term_.load(std::memory_order::memory_order_acquire)};
    std::uint64_t leaderId{this->leaderId_.load(std::memory_order::memory_order_relaxed)};
    return {term, leaderId};
}

void sharpen::RaftLeaderRecord::Flush(std::uint64_t term, std::uint64_t leaderId) noexcept {
    this->leaderId_.store(leaderId, std::memory_order::memory_order_relaxed);
    this->term_.store(term, std::memory_order::memory_order_release);
}