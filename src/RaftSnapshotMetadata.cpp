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

sharpen::RaftSnapshotMetadata::RaftSnapshotMetadata(const Self &other) noexcept
    :lastIndex_(other.lastIndex_)
    ,lastTerm_(other.lastTerm_)
{}

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

std::size_t sharpen::RaftSnapshotMetadata::ComputeSize() const noexcept
{
    sharpen::Varuint64 builder{this->lastIndex_};
    std::size_t size{builder.ComputeSize()};
    builder.Set(this->lastTerm_);
    size += builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftSnapshotMetadata::LoadFrom(const char *data,std::size_t size)
{
    std::size_t offset{0};
    if(size < 2)
    {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot metadata"};
    }
    sharpen::Varuint64 builder{0};
    offset += builder.LoadFrom(data,size);
    std::uint64_t lastIndex{builder.Get()};
    if(offset == size)
    {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot metadata"};
    }
    offset += builder.LoadFrom(data + offset,size - offset);
    std::uint64_t lastTerm{builder.Get()};
    this->lastIndex_ = lastIndex;
    this->lastTerm_ = lastTerm;
    return offset;
}

std::size_t sharpen::RaftSnapshotMetadata::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->lastIndex_};
    offset += builder.UnsafeStoreTo(data);
    builder.Set(this->lastTerm_);
    offset += builder.UnsafeStoreTo(data + offset);
    return offset;
}