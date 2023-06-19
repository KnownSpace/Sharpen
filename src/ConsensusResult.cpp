#include <sharpen/ConsensusResult.hpp>

#include <cassert>

bool sharpen::ConsensusResult::IsNone() const noexcept {
    return this->IsDefault();
}

sharpen::ConsensusResult::ConsensusResult(Base impl) noexcept
    : Base(impl) {
}

void sharpen::ConsensusResult::Set(sharpen::ConsensusResultEnum bit) noexcept {
    assert(bit != sharpen::ConsensusResultEnum::None);
    Base::Set(bit);
}

sharpen::ConsensusResult sharpen::ConsensusResult::Take() noexcept {
    return Self{Base::Take()};
}