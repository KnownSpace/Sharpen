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

void sharpen::RaftReplicatedState::Forward() noexcept
{
    if(this->snapshot_)
    {
        if(this->snapshot_->Forwardable())
        {
            this->snapshot_->Forward();
        }
        else
        {
            this->snapshot_.reset(nullptr);
            this->matchIndex_ = this->nextIndex_;
            this->nextIndex_ = this->snapshotMetadata_.GetLastIndex() + 1;
        }
    }
    else
    {
        this->nextIndex_ += 1;
    }
}

void sharpen::RaftReplicatedState::Reset() noexcept
{
    this->nextIndex_ = this->matchIndex_;
    if(this->snapshot_)
    {
        this->snapshot_.reset(nullptr);
    }
}

void sharpen::RaftReplicatedState::ForwardCommitPoint(std::uint64_t index) noexcept
{
    this->matchIndex_ = (std::max)(index,this->matchIndex_);
}

void sharpen::RaftReplicatedState::BackwardCommitPoint(std::uint64_t index) noexcept
{
    this->matchIndex_ = (std::min)(index,this->matchIndex_);
}

void sharpen::RaftReplicatedState::SetSnapshot(sharpen::IRaftSnapshot &snapshot)
{
    std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk{snapshot.GetChainedChunks()};
    assert(chunk != nullptr);
    this->snapshot_ = std::move(chunk);
    this->snapshotMetadata_ = snapshot.GetMetadata();
}

const sharpen::IRaftSnapshotChunk *sharpen::RaftReplicatedState::LookupSnapshot() noexcept
{
    return this->snapshot_.get();
}

sharpen::Optional<sharpen::RaftSnapshotMetadata> sharpen::RaftReplicatedState::LookupSnapshotMetadata() noexcept
{
    if(this->snapshot_)
    {
        assert(this->snapshotMetadata_.GetLastIndex() != 0);
        assert(this->snapshotMetadata_.GetLastTerm() != 0);
        return this->snapshotMetadata_;
    }
    return sharpen::EmptyOpt;
}