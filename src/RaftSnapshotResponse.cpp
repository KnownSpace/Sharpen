#include <sharpen/RaftSnapshotResponse.hpp>

#include <sharpen/ConsensusWriter.hpp>

sharpen::RaftSnapshotResponse::RaftSnapshotResponse() noexcept
    : status_(false)
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , leaseRound_(0) {
}

sharpen::RaftSnapshotResponse::RaftSnapshotResponse(bool status, std::uint64_t term) noexcept
    : status_(status)
    , term_(term)
    , leaseRound_(0) {
}

sharpen::RaftSnapshotResponse::RaftSnapshotResponse(Self &&other) noexcept
    : status_(other.status_)
    , term_(other.term_)
    , leaseRound_(other.leaseRound_) {
    other.status_ = false;
    other.term_ = sharpen::ConsensusWriter::noneEpoch;
    other.leaseRound_ = 0;
}

sharpen::RaftSnapshotResponse &sharpen::RaftSnapshotResponse::operator=(
    const Self &other) noexcept {
    if (this != std::addressof(other)) {
        this->status_ = other.status_;
        this->term_ = other.term_;
        this->leaseRound_ = other.leaseRound_;
    }
    return *this;
}

sharpen::RaftSnapshotResponse &sharpen::RaftSnapshotResponse::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->status_ = other.status_;
        this->term_ = other.term_;
        this->leaseRound_ = other.leaseRound_;
        other.status_ = false;
        other.term_ = sharpen::ConsensusWriter::noneEpoch;
        other.leaseRound_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftSnapshotResponse::ComputeSize() const noexcept {
    std::size_t size{sizeof(std::uint8_t)};
    size += sizeof(this->term_);
    size += sizeof(this->leaseRound_);
    return size;
}

std::size_t sharpen::RaftSnapshotResponse::LoadFrom(const char *data, std::size_t size) {
    if (size < sizeof(std::uint8_t) + sizeof(this->term_) + sizeof(this->leaseRound_)) {
        throw sharpen::CorruptedDataError{"corrupted raft snapshot response"};
    }
    std::size_t offset{0};
    std::uint8_t status{0};
    std::memcpy(&status, data, sizeof(status));
    offset += sizeof(status);
    std::uint64_t term{0};
    offset += sharpen::BinarySerializator::LoadFrom(term, data + offset, size - offset);
    offset += sharpen::BinarySerializator::LoadFrom(this->leaseRound_,data + offset,size - offset);
    this->status_ = status;
    this->term_ = term;
    return offset;
}

std::size_t sharpen::RaftSnapshotResponse::UnsafeStoreTo(char *data) const noexcept {
    std::size_t offset{0};
    std::uint8_t status{this->status_};
    std::memcpy(data, &status, sizeof(status));
    offset += sizeof(status);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->term_, data + offset);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->leaseRound_,data + offset);
    return offset;
}