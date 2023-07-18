#include <sharpen/RaftHeartbeatResponse.hpp>

#include <sharpen/ConsensusWriter.hpp>
#include <sharpen/Varint.hpp>


sharpen::RaftHeartbeatResponse::RaftHeartbeatResponse(bool status) noexcept
    : status_(static_cast<std::uint8_t>(status))
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , matchIndex_(0)
    , leaseRound_(0) {
}

sharpen::RaftHeartbeatResponse::RaftHeartbeatResponse(Self &&other) noexcept
    : status_(other.status_)
    , term_(other.term_)
    , matchIndex_(other.matchIndex_)
    , leaseRound_(other.leaseRound_) {
    other.status_ = 0;
    other.term_ = sharpen::ConsensusWriter::noneEpoch;
    other.matchIndex_ = 0;
    other.leaseRound_ = 0;
}

sharpen::RaftHeartbeatResponse &sharpen::RaftHeartbeatResponse::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->status_ = other.status_;
        this->term_ = other.term_;
        this->matchIndex_ = other.matchIndex_;
        this->leaseRound_ = other.leaseRound_;
        other.status_ = 0;
        other.term_ = sharpen::ConsensusWriter::noneEpoch;
        other.matchIndex_ = 0;
        other.leaseRound_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftHeartbeatResponse::ComputeSize() const noexcept {
    std::size_t size{sizeof(this->status_)};
    sharpen::Varuint64 builder{this->term_};
    size += builder.ComputeSize();
    builder.Set(this->matchIndex_);
    size += builder.ComputeSize();
    builder.Set(this->leaseRound_);
    size += builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftHeartbeatResponse::LoadFrom(const char *data, std::size_t size) {
    if (size < 4) {
        throw sharpen::CorruptedDataError{"corrupted heartbeat response"};
    }
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::LoadFrom(this->status_, data + offset, size - offset);
    if (size < 3 + offset) {
        throw sharpen::CorruptedDataError{"corrupted heartbeat response"};
    }
    sharpen::Varuint64 builder{0};
    offset += sharpen::BinarySerializator::LoadFrom(builder, data + offset, size - offset);
    this->term_ = builder.Get();
    if (size < 2 + offset) {
        throw sharpen::CorruptedDataError{"corrupted heartbeat response"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder, data + offset, size - offset);
    this->matchIndex_ = builder.Get();
    if (size < 1 + offset) {
        throw sharpen::CorruptedDataError{"corrupted heartbeat response"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder, data + offset, size - offset);
    this->leaseRound_ = builder.Get();
    return offset;
}

std::size_t sharpen::RaftHeartbeatResponse::UnsafeStoreTo(char *data) const noexcept {
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->status_, data + offset);
    sharpen::Varuint64 builder{this->term_};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder, data + offset);
    builder.Set(this->matchIndex_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder, data + offset);
    builder.Set(this->leaseRound_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    return offset;
}