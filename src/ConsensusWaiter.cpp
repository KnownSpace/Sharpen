#include <sharpen/ConsensusWaiter.hpp>

sharpen::ConsensusWaiter::ConsensusWaiter() noexcept
    : index_(0)
    , future_(nullptr) {
}

sharpen::ConsensusWaiter::ConsensusWaiter(std::uint64_t index,
                                          sharpen::Future<std::uint64_t> &future) noexcept
    : index_(index)
    , future_(&future) {
}

sharpen::ConsensusWaiter::ConsensusWaiter(Self &&other) noexcept
    : index_(other.index_)
    , future_(other.future_) {
    other.index_ = 0;
    other.future_ = nullptr;
}

sharpen::ConsensusWaiter &sharpen::ConsensusWaiter::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->index_ = other.index_;
        this->future_ = other.future_;
        other.index_ = 0;
        other.future_ = nullptr;
    }
    return *this;
}

std::int32_t sharpen::ConsensusWaiter::CompareWith(const Self &other) const noexcept {
    if (this->index_ > other.index_) {
        return 1;
    } else if (this->index_ < other.index_) {
        return -1;
    }
    return 0;
}