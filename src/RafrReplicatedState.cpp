#include <sharpen/RaftReplicatedState.hpp>

#include <cassert>

sharpen::RaftReplicatedState::RaftReplicatedState() noexcept
    :Self{0}
{}

sharpen::RaftReplicatedState::RaftReplicatedState(std::uint64_t matchIndex) noexcept
    :matchIndex_(matchIndex)
    ,nextIndex_(matchIndex)
    ,snapshot_(nullptr)
{}

void sharpen::RaftReplicatedState::Forward(std::uint64_t step) noexcept
{
    if(this->snapshot_)
    {
        assert(step == 1);
        if(this->snapshot_->Forwardable())
        {
            this->snapshot_->Forward();
        }
        else
        {
            this->snapshot_.reset(nullptr);
            this->matchIndex_ = this->snapshotMetadata_.GetLastIndex();
            this->nextIndex_ = this->matchIndex_ + 1;
        }
    }
    else
    {
        this->nextIndex_ += step;
    }
}

void sharpen::RaftReplicatedState::Forward() noexcept
{
    return this->Forward(1);
}

void sharpen::RaftReplicatedState::ForwardMatchPoint(std::uint64_t index) noexcept
{
    if(index > this->matchIndex_)
    {
        this->matchIndex_ = index;
        if(index > this->nextIndex_)
        {
            this->nextIndex_ = this->matchIndex_ + 1;
        }
    }
}

void sharpen::RaftReplicatedState::BackwardMatchPoint(std::uint64_t index) noexcept
{
    if(index <= this->matchIndex_)
    {
        this->matchIndex_ = index;
        this->nextIndex_ = this->matchIndex_ + 1;
    }
}

void sharpen::RaftReplicatedState::SetSnapshot(sharpen::RaftSnapshot snapshot) noexcept
{
    std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk{snapshot.ReleaseChainedChunks()};
    assert(chunk != nullptr);
    this->snapshot_ = std::move(chunk);
    this->snapshotMetadata_ = snapshot.Metadata();
}

const sharpen::IRaftSnapshotChunk *sharpen::RaftReplicatedState::LookupSnapshot() const noexcept
{
    return this->snapshot_.get();
}

sharpen::IRaftSnapshotChunk *sharpen::RaftReplicatedState::LookupSnapshot() noexcept
{
    return this->snapshot_.get();
}

sharpen::Optional<sharpen::RaftSnapshotMetadata> sharpen::RaftReplicatedState::LookupSnapshotMetadata() const noexcept
{
    if(this->snapshot_)
    {
        assert(this->snapshotMetadata_.GetLastIndex() != 0);
        assert(this->snapshotMetadata_.GetLastTerm() != 0);
        return this->snapshotMetadata_;
    }
    return sharpen::EmptyOpt;
}