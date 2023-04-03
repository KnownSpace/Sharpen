#include <sharpen/RaftSnapshotMetadata.hpp>

sharpen::RaftSnapshotMetadata::RaftSnapshotMetadata() noexcept
    :lastIndex_(0)
    ,lastTerm_(0)
{}

sharpen::RaftSnapshotMetadata::RaftSnapshotMetadata(std::uint64_t index,std::uint64_t term) noexcept
    :lastIndex_(index)
    ,lastTerm_(0)
{
    if(this->lastIndex_)
    {
        this->lastTerm_ = term;
    }
}

sharpen::RaftSnapshotMetadata::RaftSnapshotMetadata(Self &&other) noexcept
    :lastIndex_(other.lastIndex_)
    ,lastTerm_(other.lastTerm_)
{
    other.lastIndex_ = 0;
    other.lastTerm_ = 0;
}

sharpen::RaftSnapshotMetadata &sharpen::RaftSnapshotMetadata::operator=(const Self &other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lastIndex_ = other.lastIndex_;
        this->lastTerm_ = other.lastTerm_;
    }
    return *this;
}

sharpen::RaftSnapshotMetadata &sharpen::RaftSnapshotMetadata::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lastIndex_ = other.lastIndex_;
        this->lastTerm_ = other.lastTerm_;
        other.lastIndex_ = 0;
        other.lastTerm_ = 0;
    }
    return *this;
}

void sharpen::RaftSnapshotMetadata::SetLastIndex(std::uint64_t index) noexcept
{
    this->lastIndex_ = index;
    if(!this->lastIndex_)
    {
        this->lastTerm_ = 0;
    }
}