#include <sharpen/RaftVoteForRequest.hpp>

#include <sharpen/Varint.hpp>
#include <sharpen/ConsensusWriter.hpp>

sharpen::RaftVoteForRequest::RaftVoteForRequest() noexcept
    : actorId_()
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , lastIndex_(0)
    , lastTerm_(0)
    , leaderCount_(0) {
}

sharpen::RaftVoteForRequest::RaftVoteForRequest(const sharpen::ActorId &actorId,
                                                std::uint64_t term,
                                                std::uint64_t lastIndex,
                                                std::uint64_t lastTerm) noexcept
    : actorId_(actorId)
    , term_(term)
    , lastIndex_(lastIndex)
    , lastTerm_(lastTerm)
    , leaderCount_(0) {
}

sharpen::RaftVoteForRequest::RaftVoteForRequest(Self &&other) noexcept
    : actorId_(std::move(other.actorId_))
    , term_(other.term_)
    , lastIndex_(other.lastIndex_)
    , lastTerm_(other.lastTerm_)
    , leaderCount_(other.leaderCount_) {
    other.term_ = sharpen::ConsensusWriter::noneEpoch;
    other.lastIndex_ = 0;
    other.lastTerm_ = 0;
    other.leaderCount_ = 0;
}

sharpen::RaftVoteForRequest &sharpen::RaftVoteForRequest::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->actorId_ = std::move(other.actorId_);
        this->term_ = other.term_;
        this->lastIndex_ = other.lastIndex_;
        this->lastTerm_ = other.lastTerm_;
        this->leaderCount_ = other.leaderCount_;
        other.term_ = sharpen::ConsensusWriter::noneEpoch;
        other.lastIndex_ = 0;
        other.lastTerm_ = 0;
        other.leaderCount_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftVoteForRequest::ComputeSize() const noexcept {
    sharpen::Varuint64 builder{0};
    std::size_t size{sharpen::BinarySerializator::ComputeSize(this->actorId_)};
    builder.Set(this->GetTerm());
    size += builder.ComputeSize();
    builder.Set(this->GetLastIndex());
    size += builder.ComputeSize();
    builder.Set(this->GetLastTerm());
    size += builder.ComputeSize();
    builder.Set(this->leaderCount_);
    size += builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftVoteForRequest::LoadFrom(const char *data, std::size_t size) {
    if (size < 4 + sizeof(sharpen::ActorId)) {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    std::size_t offset{0};
    sharpen::Varuint64 builder{0};
    offset += sharpen::BinarySerializator::LoadFrom(this->actorId_, data, size);
    if (size < 4 + offset) {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset, size - offset);
    this->term_ = builder.Get();
    if (size < 3 + offset) {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset, size - offset);
    this->lastIndex_ = builder.Get();
    if (size < 2 + offset) {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset, size - offset);
    this->lastTerm_ = builder.Get();
    if (size < 1 + offset) {
        throw sharpen::CorruptedDataError{"corrupted vote request"};
    }
    offset += builder.LoadFrom(data + offset, size - offset);
    this->leaderCount_ = builder.Get();
    return offset;
}

std::size_t sharpen::RaftVoteForRequest::UnsafeStoreTo(char *data) const noexcept {
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->GetTerm()};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->actorId_, data);
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->GetLastIndex());
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->GetLastTerm());
    offset += builder.UnsafeStoreTo(data + offset);
    builder.Set(this->leaderCount_);
    offset += builder.UnsafeStoreTo(data + offset);
    return offset;
}