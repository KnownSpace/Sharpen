#include <sharpen/RaftSnapshot.hpp>

sharpen::RaftSnapshot::RaftSnapshot(std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk) noexcept
    : chunk_(std::move(chunk))
    , metadata_(0, 0)
{
}

sharpen::RaftSnapshot::RaftSnapshot(std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk,
                                    sharpen::RaftSnapshotMetadata metadata) noexcept
    : chunk_(std::move(chunk))
    , metadata_(metadata)
{
}

sharpen::RaftSnapshot::RaftSnapshot(Self &&other) noexcept
    : chunk_(std::move(other.chunk_))
    , metadata_(std::move(other.metadata_))
{
}

sharpen::RaftSnapshot &sharpen::RaftSnapshot::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->chunk_ = std::move(other.chunk_);
        this->metadata_ = std::move(other.metadata_);
    }
    return *this;
}

std::unique_ptr<sharpen::IRaftSnapshotChunk> sharpen::RaftSnapshot::ReleaseChainedChunks() noexcept
{
    return std::move(this->chunk_);
}