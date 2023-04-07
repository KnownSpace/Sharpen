#include <sharpen/RaftSnapshotRequest.hpp>

sharpen::RaftSnapshotRequest::RaftSnapshotRequest() noexcept
    :metadata_(0,0)
    ,last_(false)
    ,data_()
{}

sharpen::RaftSnapshotRequest::RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata) noexcept
    :Self{metadata,sharpen::ByteBuffer{}}
{}

sharpen::RaftSnapshotRequest::RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata,sharpen::ByteBuffer data) noexcept
    :Self{metadata,std::move(data),false}
{}

sharpen::RaftSnapshotRequest::RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata,sharpen::ByteBuffer data,bool last) noexcept
    :metadata_(metadata)
    ,last_(last)
    ,data_(std::move(data))
{}

sharpen::RaftSnapshotRequest::RaftSnapshotRequest(Self &&other) noexcept
    :metadata_(std::move(other.metadata_))
    ,last_(other.last_)
    ,data_(std::move(other.data_))
{
    other.last_ = false;
}

sharpen::RaftSnapshotRequest &sharpen::RaftSnapshotRequest::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->metadata_ = std::move(other.metadata_);
        this->last_ = other.last_;
        this->data_ = std::move(other.data_);
        other.last_ = false;
    }
    return *this;
}

std::size_t sharpen::RaftSnapshotRequest::ComputeSize() const noexcept
{
    std::size_t size{sizeof(this->metadata_)};
    size += sharpen::BinarySerializator::ComputeSize(this->data_);
    size += sizeof(std::uint8_t);
    return size;
}

std::size_t sharpen::RaftSnapshotRequest::LoadFrom(const char *data,std::size_t size)
{
    if(size < sizeof(std::uint8_t) + sizeof(this->metadata_) + 1)
    {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    sharpen::RaftSnapshotMetadata metadata;
    std::size_t offset{0};
    std::memcpy(&metadata,data,sizeof(metadata));
    offset += sizeof(metadata);
    std::uint8_t last{0};
    std::memcpy(&last,data + offset,sizeof(last));
    offset += sizeof(last);
    sharpen::ByteBuffer chunkdata;
    offset += sharpen::BinarySerializator::LoadFrom(chunkdata,data + offset,size - offset);
    this->metadata_ = metadata;
    this->last_ = last;
    this->data_ = std::move(chunkdata);
    return offset;
}

std::size_t sharpen::RaftSnapshotRequest::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint8_t last{0};
    if(this->last_)
    {
        last = 1;
    }
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->metadata_,data);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(last,data + offset);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->data_,data + offset);
    return offset;
}