#include <sharpen/RaftReplicatedState.hpp>

#include <sharpen/ILogStorage.hpp>
#include <cassert>


sharpen::RaftReplicatedState::RaftReplicatedState() noexcept
    : Self{sharpen::ILogStorage::noneIndex} {
}

sharpen::RaftReplicatedState::RaftReplicatedState(std::uint64_t matchIndex) noexcept
    : matchIndex_(matchIndex)
    , nextIndex_(matchIndex + 1)
    , snapshot_(nullptr) {
}

sharpen::RaftReplicatedState::RaftReplicatedState(Self &&other) noexcept
    : matchIndex_(other.matchIndex_)
    , nextIndex_(other.nextIndex_)
    , snapshot_(std::move(other.snapshot_))
    , snapshotMetadata_(std::move(other.snapshotMetadata_)) {
    other.matchIndex_ = sharpen::ILogStorage::noneIndex;
    other.nextIndex_ = sharpen::ILogStorage::noneIndex;
}

sharpen::RaftReplicatedState &sharpen::RaftReplicatedState::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->matchIndex_ = other.matchIndex_;
        this->nextIndex_ = other.nextIndex_;
        this->snapshot_ = std::move(other.snapshot_);
        this->snapshotMetadata_ = std::move(other.snapshotMetadata_);
        other.matchIndex_ = sharpen::ILogStorage::noneIndex;
        other.nextIndex_ = sharpen::ILogStorage::noneIndex;
    }
    return *this;
}

void sharpen::RaftReplicatedState::Forward(std::uint64_t step) noexcept {
    if (this->snapshot_) {
        assert(step == 1);
        if (this->snapshot_->Forwardable()) {
            this->snapshot_->Forward();
        } else {
            this->snapshot_.reset(nullptr);
            this->matchIndex_ = this->snapshotMetadata_.GetLastIndex();
            this->nextIndex_ = this->matchIndex_ + 1;
        }
    } else {
        this->nextIndex_ += step;
    }
}

void sharpen::RaftReplicatedState::Forward() noexcept {
    return this->Forward(1);
}

void sharpen::RaftReplicatedState::ForwardMatchPoint(std::uint64_t index) noexcept {
    if (index > this->matchIndex_) {
        this->matchIndex_ = index;
        if (index >= this->nextIndex_) {
            this->nextIndex_ = this->matchIndex_ + 1;
        }
    }
}

void sharpen::RaftReplicatedState::BackwardMatchPoint(std::uint64_t index) noexcept {
    if (index <= this->matchIndex_) {
        this->matchIndex_ = index;
        this->nextIndex_ = this->matchIndex_ + 1;
    }
}

void sharpen::RaftReplicatedState::SetSnapshot(sharpen::RaftSnapshot snapshot) noexcept {
    std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk{snapshot.ReleaseChainedChunks()};
    assert(chunk != nullptr);
    this->snapshot_ = std::move(chunk);
    this->snapshotMetadata_ = snapshot.Metadata();
}

const sharpen::IRaftSnapshotChunk *sharpen::RaftReplicatedState::LookupSnapshot() const noexcept {
    return this->snapshot_.get();
}

sharpen::IRaftSnapshotChunk *sharpen::RaftReplicatedState::LookupSnapshot() noexcept {
    return this->snapshot_.get();
}

sharpen::Optional<sharpen::RaftSnapshotMetadata>
sharpen::RaftReplicatedState::LookupSnapshotMetadata() const noexcept {
    if (this->snapshot_) {
        assert(this->snapshotMetadata_.GetLastIndex() != sharpen::ILogStorage::noneIndex);
        assert(this->snapshotMetadata_.GetLastTerm() != sharpen::ILogStorage::noneIndex);
        return this->snapshotMetadata_;
    }
    return sharpen::EmptyOpt;
}