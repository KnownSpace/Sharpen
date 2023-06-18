#include <sharpen/RaftOption.hpp>

#include <algorithm>

sharpen::RaftOption::RaftOption() noexcept
    : isLearner_(false)
    , enablePrevote_(false)
    , batchSize_(Self::minBatchSize_)
    , pipelineLength_(Self::minPipelineLength_)
    , enableSingle_(false) {
}

sharpen::RaftOption::RaftOption(Self &&other) noexcept
    : isLearner_(other.isLearner_)
    , enablePrevote_(other.enablePrevote_)
    , batchSize_(other.batchSize_)
    , pipelineLength_(other.pipelineLength_)
    , enableSingle_(other.enableSingle_) {
    other.isLearner_ = false;
    other.enablePrevote_ = false;
    other.batchSize_ = Self::minBatchSize_;
    other.pipelineLength_ = Self::minPipelineLength_;
    other.enableSingle_ = false;
}

sharpen::RaftOption &sharpen::RaftOption::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->isLearner_ = other.isLearner_;
        this->enablePrevote_ = other.enablePrevote_;
        this->batchSize_ = other.batchSize_;
        this->pipelineLength_ = other.pipelineLength_;
        this->enableSingle_ = other.enableSingle_;
        other.isLearner_ = false;
        other.enablePrevote_ = false;
        other.batchSize_ = Self::minBatchSize_;
        other.pipelineLength_ = Self::minPipelineLength_;
        other.enableSingle_ = false;
    }
    return *this;
}

void sharpen::RaftOption::SetBatchSize(std::uint32_t batchSize) noexcept {
    if (batchSize > Self::maxBatchSize_) {
        batchSize = Self::maxBatchSize_;
    }
    if (batchSize < Self::minBatchSize_) {
        batchSize = Self::minBatchSize_;
    }
    this->batchSize_ = batchSize;
}

void sharpen::RaftOption::SetPipelineLength(std::uint32_t length) noexcept {
    if (length > Self::maxPipelineLength_) {
        length = Self::maxPipelineLength_;
    }
    if (length < Self::minPipelineLength_) {
        length = Self::minPipelineLength_;
    }
    this->pipelineLength_ = length;
}