#include <sharpen/RaftLeaderCounter.hpp>

sharpen::RaftLeaderCounter::RaftLeaderCounter() noexcept
    : count_(0) {
}

sharpen::RaftLeaderCounter::RaftLeaderCounter(const Self &other) noexcept
    : count_(other.count_.load()) {
}

sharpen::RaftLeaderCounter::RaftLeaderCounter(Self &&other) noexcept
    : count_(other.count_.exchange(0)) {
}

sharpen::RaftLeaderCounter &sharpen::RaftLeaderCounter::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->count_ = other.count_.exchange(0);
    }
    return *this;
}

std::uint64_t sharpen::RaftLeaderCounter::GetCurrentCount() const noexcept {
    return this->count_;
}

bool sharpen::RaftLeaderCounter::TryComeToPower(std::uint64_t prevCount) noexcept {
    std::uint64_t tmp{prevCount};
    while (!this->count_.compare_exchange_strong(tmp,tmp + 1)) {
        tmp = this->count_.load();
        if (tmp > prevCount) {
            return false;
        }
    }
    return true;
}

void sharpen::RaftLeaderCounter::Abdicate() noexcept {
    this->count_ -= 1;
}