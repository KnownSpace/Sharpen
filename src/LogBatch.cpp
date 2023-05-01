#include <sharpen/LogBatch.hpp>

sharpen::LogBatch &sharpen::LogBatch::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->entires_ = std::move(other.entires_);
    }
    return *this;
}

void sharpen::LogBatch::Append(sharpen::ByteBuffer log) {
    this->Reverse(1);
    this->entires_.emplace_back(std::move(log));
}

std::size_t sharpen::LogBatch::GetSize() const noexcept {
    return this->entires_.size();
}

sharpen::ByteBuffer &sharpen::LogBatch::Get(std::size_t index) noexcept {
    assert(index < this->GetSize());
    return this->entires_[index];
}

const sharpen::ByteBuffer &sharpen::LogBatch::Get(std::size_t index) const noexcept {
    assert(index < this->GetSize());
    return this->entires_[index];
}

void sharpen::LogBatch::Reverse(std::size_t size) {
    assert(size != 0);
    this->entires_.reserve(this->GetSize() + size);
}

bool sharpen::LogBatch::Empty() const noexcept {
    return this->entires_.empty();
}