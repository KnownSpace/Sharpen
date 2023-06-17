#include <sharpen/RaftSnapshotRequest.hpp>

sharpen::RaftSnapshotRequest::RaftSnapshotRequest() noexcept
    : term_(0)
    , leaderActorId_()
    , offset_(0)
    , last_(false)
    , metadata_(0, 0)
    , data_() {
}

sharpen::RaftSnapshotRequest::RaftSnapshotRequest(Self &&other) noexcept
    : term_(other.term_)
    , leaderActorId_(std::move(other.leaderActorId_))
    , offset_(other.offset_)
    , last_(other.last_)
    , metadata_(std::move(other.metadata_))
    , data_(std::move(other.data_)) {
    other.term_ = 0;
    other.offset_ = 0;
    other.last_ = false;
}

sharpen::RaftSnapshotRequest &sharpen::RaftSnapshotRequest::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->term_ = other.term_;
        this->leaderActorId_ = std::move(other.leaderActorId_);
        this->offset_ = other.offset_;
        this->last_ = other.last_;
        this->metadata_ = std::move(other.metadata_);
        this->data_ = std::move(other.data_);
        other.term_ = 0;
        other.offset_ = 0;
        other.last_ = false;
    }
    return *this;
}

std::size_t sharpen::RaftSnapshotRequest::ComputeSize() const noexcept {
    std::size_t size{0};
    sharpen::Varuint64 builder{this->term_};
    size += builder.ComputeSize();
    size += sharpen::BinarySerializator::ComputeSize(this->leaderActorId_);
    builder.Set(this->offset_);
    size += builder.ComputeSize();
    size += sizeof(std::uint8_t);
    size += this->metadata_.ComputeSize();
    size += sharpen::BinarySerializator::ComputeSize(this->data_);
    return size;
}

std::size_t sharpen::RaftSnapshotRequest::LoadFrom(const char *data, std::size_t size) {
    if (size < 6 + sizeof(sharpen::ActorId)) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    std::size_t offset{0};
    sharpen::Varuint64 builder{0};
    offset += builder.LoadFrom(data, size);
    std::uint64_t term{builder.Get()};
    if (size < 5 + sizeof(sharpen::ActorId) + offset) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    offset +=
        sharpen::BinarySerializator::LoadFrom(this->leaderActorId_, data + offset, size - offset);
    if (size < 5 + offset) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    offset += builder.LoadFrom(data + offset, size - offset);
    std::uint64_t off{builder.Get()};
    if (size < 4 + offset) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    std::uint8_t last{0};
    std::memcpy(&last, data + offset, sizeof(last));
    offset += sizeof(last);
    if (size < 3 + offset) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    sharpen::RaftSnapshotMetadata metadata;
    offset += sharpen::BinarySerializator::LoadFrom(metadata, data, size);
    if (offset == size) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot request"};
    }
    sharpen::ByteBuffer chunkdata;
    offset += sharpen::BinarySerializator::LoadFrom(chunkdata, data + offset, size - offset);
    this->term_ = term;
    this->offset_ = off;
    this->last_ = last;
    this->metadata_ = metadata;
    this->data_ = std::move(chunkdata);
    return offset;
}

std::size_t sharpen::RaftSnapshotRequest::UnsafeStoreTo(char *data) const noexcept {
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->term_};
    offset += builder.UnsafeStoreTo(data);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->leaderActorId_, data + offset);
    builder.Set(this->offset_);
    offset += builder.UnsafeStoreTo(data + offset);
    std::uint8_t last{0};
    if (this->last_) {
        last = 1;
    }
    offset += sharpen::BinarySerializator::UnsafeStoreTo(last, data + offset);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->metadata_, data);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->data_, data + offset);
    return offset;
}