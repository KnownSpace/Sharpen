#include <sharpen/ConsensusTask.hpp>

sharpen::ConsensusTask::ConsensusTask(std::uint64_t index,
                                      sharpen::Future<sharpen::ConsensusStatus> &future) noexcept
    : index_(index)
    , future_(&future) {
}

sharpen::ConsensusTask::ConsensusTask(Self &&other) noexcept
    : index_(other.index_)
    , future_(other.future_) {
    other.index_ = 0;
    other.future_ = nullptr;
}

sharpen::ConsensusTask &sharpen::ConsensusTask::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->index_ = other.index_;
        this->future_ = other.future_;
        other.index_ = 0;
        other.future_ = nullptr;
    }
    return *this;
}

bool sharpen::ConsensusTask::Vaild() const noexcept {
    return this->index_ && this->future_;
}

void sharpen::ConsensusTask::Complete(sharpen::ConsensusStatus status) {
    if (this->future_) {
        this->future_->Complete(status);
    }
}

std::int32_t sharpen::ConsensusTask::CompareWith(const Self &other) const noexcept {
    if (this->index_ < other.index_) {
        return -1;
    } else if (this->index_ > other.index_) {
        return 1;
    }
    if (this->future_ < other.future_) {
        return -1;
    } else if (this->future_ > other.future_) {
        return 1;
    }
    return 0;
}