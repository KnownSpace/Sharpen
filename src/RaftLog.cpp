#include <sharpen/RaftLog.hpp>

#include <sharpen/Varint.hpp>

sharpen::RaftLog::RaftLog() noexcept
    :term_(0)
    ,content_()
{}

sharpen::RaftLog::RaftLog(std::uint64_t term) noexcept
    :term_(term)
    ,content_()
{}

sharpen::RaftLog::RaftLog(std::uint64_t term,sharpen::ByteBuffer content) noexcept
    :term_(term)
    ,content_(std::move(content))
{}

sharpen::RaftLog::RaftLog(Self &&other) noexcept
    :term_(other.term_)
    ,content_(std::move(other.content_))
{
    other.term_ = 0;
}

sharpen::RaftLog &sharpen::RaftLog::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->term_ = other.term_;
        this->content_ = std::move(other.content_);
        other.term_ = 0;
    }
    return *this;
}

std::size_t sharpen::RaftLog::ComputeSize() const noexcept
{
    std::size_t size{0};
    sharpen::Varuint64 builder{this->term_};
    size += builder.ComputeSize();
    size += this->content_.ComputeSize();
    return size;
}

std::size_t sharpen::RaftLog::LoadFrom(const char *data,std::size_t size)
{
    if(size < 2)
    {
        throw sharpen::CorruptedDataError{"corrupted raft log"};
    }
    std::size_t offset{0};
    sharpen::Varuint64 builder{0};
    offset += builder.LoadFrom(data,size);
    if(size < offset + 1)
    {
        throw sharpen::CorruptedDataError{"corrupted raft log"};
    }
    offset += this->content_.LoadFrom(data + offset,size - offset);
    return offset;
}

std::size_t sharpen::RaftLog::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->term_};
    offset += builder.UnsafeStoreTo(data);
    offset += this->content_.UnsafeStoreTo(data);
    return offset;
}