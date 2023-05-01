#include <sharpen/WriteLogsResult.hpp>

#include <sharpen/IntOps.hpp>
#include <cassert>

sharpen::WriteLogsResult::WriteLogsResult(std::uint64_t lastIndex) noexcept
    : lastIndex_(lastIndex)
    , beginIndex_(0) {
}

sharpen::WriteLogsResult::WriteLogsResult(std::uint64_t lastIndex,
                                          std::uint64_t beginIndex) noexcept
    : lastIndex_(lastIndex)
    , beginIndex_(beginIndex) {
    assert(this->lastIndex_ >= this->beginIndex_);
}

sharpen::WriteLogsResult::WriteLogsResult(Self &&other) noexcept
    : lastIndex_(other.lastIndex_)
    , beginIndex_(other.beginIndex_) {
    other.lastIndex_ = 0;
    other.beginIndex_ = 0;
}

sharpen::WriteLogsResult &sharpen::WriteLogsResult::operator=(const Self &other) noexcept {
    if (this != std::addressof(other)) {
        this->lastIndex_ = other.lastIndex_;
        this->beginIndex_ = other.beginIndex_;
    }
    return *this;
}

sharpen::WriteLogsResult &sharpen::WriteLogsResult::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->lastIndex_ = other.lastIndex_;
        this->beginIndex_ = other.beginIndex_;
        other.lastIndex_ = 0;
        other.beginIndex_ = 0;
    }
    return *this;
}

bool sharpen::WriteLogsResult::GetStatus() const noexcept {
    return this->beginIndex_;
}

std::uint64_t sharpen::WriteLogsResult::GetLastIndex() const noexcept {
    return this->lastIndex_;
}

sharpen::Optional<std::uint64_t> sharpen::WriteLogsResult::LookupBeginIndex() const noexcept {
    if (this->beginIndex_) {
        return this->beginIndex_;
    }
    return sharpen::EmptyOpt;
}

std::size_t sharpen::WriteLogsResult::GetWrittenSize() const noexcept {
    if (this->beginIndex_) {
        return sharpen::IntCast<std::size_t>(this->lastIndex_ - this->beginIndex_ + 1);
    }
    return 0;
}