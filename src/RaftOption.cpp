#include <sharpen/RaftOption.hpp>

#include <algorithm>

sharpen::RaftOption::RaftOption() noexcept
    : isLearner_(false)
    , enablePrevote_(false)
    , batchSize_(1) {
}

sharpen::RaftOption::RaftOption(Self &&other) noexcept
    : isLearner_(other.isLearner_)
    , enablePrevote_(other.enablePrevote_)
    , batchSize_(other.batchSize_) {
    other.isLearner_ = false;
    other.enablePrevote_ = false;
    other.batchSize_ = 1;
}

sharpen::RaftOption &sharpen::RaftOption::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->isLearner_ = other.isLearner_;
        this->enablePrevote_ = other.enablePrevote_;
        this->batchSize_ = other.batchSize_;
        other.isLearner_ = false;
        other.enablePrevote_ = false;
        other.batchSize_ = 1;
    }
    return *this;
}

void sharpen::RaftOption::SetBatchSize(std::uint32_t batchSize) noexcept {
    batchSize = (std::max)(Self::minBatchSize_, batchSize);
    batchSize = (std::min)(Self::maxBatchSize_, batchSize);
    this->batchSize_ = batchSize;
}