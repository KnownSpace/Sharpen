#include <sharpen/RaftHeartbeatRequest.hpp>

#include <sharpen/BufferWriter.hpp>
#include <sharpen/BufferReader.hpp>
#include <sharpen/Varint.hpp>

sharpen::RaftHeartbeatRequest::RaftHeartbeatRequest() noexcept
    :term_(0)
    ,leaderId_(0)
    ,preLogIndex_(0)
    ,preLogTerm_(0)
    ,entries_()
    ,leaderCommitIndex_(0)
{}

sharpen::RaftHeartbeatRequest::RaftHeartbeatRequest(Self &&other) noexcept
    :term_(other.term_)
    ,leaderId_(other.leaderId_)
    ,preLogIndex_(other.preLogIndex_)
    ,preLogTerm_(other.preLogTerm_)
    ,entries_(std::move(other.entries_))
    ,leaderCommitIndex_(other.leaderCommitIndex_)
{
    other.term_ = 0;
    other.leaderId_ = 0;
    other.preLogIndex_ = 0;
    other.preLogTerm_ = 0;
    other.leaderCommitIndex_ = 0;
}

sharpen::RaftHeartbeatRequest &sharpen::RaftHeartbeatRequest::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->term_ = other.term_;
        this->leaderId_ = other.leaderId_;
        this->preLogIndex_ = other.preLogIndex_;
        this->preLogTerm_ = other.preLogTerm_;
        this->entries_ = std::move(other.entries_);
        this->leaderCommitIndex_ = other.leaderCommitIndex_;
        other.term_ = 0;
        other.leaderId_ = 0;
        other.preLogIndex_ = 0;
        other.preLogTerm_ = 0;
        other.leaderCommitIndex_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftHeartbeatRequest::ComputeSize() const noexcept
{
    std::size_t size{0};
    sharpen::Varuint64 builder{this->term_};
    size += builder.ComputeSize();
    builder.Set(this->leaderId_);
    size += builder.ComputeSize();
    builder.Set(this->preLogIndex_);
    size += builder.ComputeSize();
    builder.Set(this->preLogTerm_);
    size += builder.ComputeSize();
    size += sharpen::BinarySerializator::ComputeSize(this->entries_);
    builder.Set(this->leaderCommitIndex_);
    size+= builder.ComputeSize();
    return size;
}

std::size_t sharpen::RaftHeartbeatRequest::LoadFrom(const char *data,std::size_t size)
{
    if(size < 6)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    sharpen::Varuint64 builder{0};
    std::size_t offset{0};
    offset += sharpen::BinarySerializator::LoadFrom(builder,data + offset,size - offset);
    this->term_ = builder.Get();
    if(size < offset + 5)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder,data + offset,size - offset);
    this->leaderId_ = builder.Get();
    if(size < offset + 4)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder,data + offset,size - offset);
    this->preLogIndex_ = builder.Get();
    if(size < offset + 3)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder,data + offset,size - offset);
    this->preLogTerm_ = builder.Get();
    sharpen::LogEntries entries;
    if(size < offset + 2)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(entries,data + offset,size - offset);
    this->entries_ = std::move(entries);
    if(size < offset + 1)
    {
        throw sharpen::CorruptedDataError{"corrupted heartbeat request"};
    }
    offset += sharpen::BinarySerializator::LoadFrom(builder,data + offset,size - offset);
    this->leaderCommitIndex_ = builder.Get();
    return offset;
}

std::size_t sharpen::RaftHeartbeatRequest::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->term_};
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    builder.Set(this->leaderId_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    builder.Set(this->preLogIndex_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    builder.Set(this->preLogTerm_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(this->entries_,data + offset);
    builder.Set(this->leaderCommitIndex_);
    offset += sharpen::BinarySerializator::UnsafeStoreTo(builder,data + offset);
    return offset;
}