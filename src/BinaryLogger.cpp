#include <sharpen/BinaryLogger.hpp>

#include <sharpen/FileOps.hpp>
#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::BinaryLogger::BinaryLogger(sharpen::FileChannelPtr channel)
    :channel_(channel)
    ,offset_(0)
{
    this->offset_ = this->channel_->GetFileSize();
}

sharpen::BinaryLogger &sharpen::BinaryLogger::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->channel_ = std::move(other.channel_);
        this->offset_.store(other.offset_);
    }
    return *this;
}

void sharpen::BinaryLogger::Clear()
{
    this->channel_->Truncate();
}

void sharpen::BinaryLogger::Log(const sharpen::WriteBatch &batch)
{
    sharpen::ByteBuffer buf{batch.ComputeSize() + sizeof(sharpen::Uint64)};
    batch.UnsafeStoreTo(buf.Data() + sizeof(sharpen::Uint64));
    sharpen::Uint64 size{buf.GetSize()};
    sharpen::Uint64 offset{0};
    try
    {
        offset = this->offset_.fetch_add(size,std::memory_order::memory_order_relaxed);
        this->channel_->WriteAsync(buf,offset);
    }
    catch(const std::exception&)
    {
        this->channel_->Truncate(offset);
        throw;
    }
}

std::vector<sharpen::WriteBatch> sharpen::BinaryLogger::GetWriteBatchs()
{
    std::vector<sharpen::WriteBatch> batchs;
    sharpen::Uint64 offset{0};
    sharpen::ByteBuffer buf;
    while (offset != this->offset_)
    {
        sharpen::WriteBatch batch;
        sharpen::Uint64 size{0};
        this->channel_->ReadAsync(reinterpret_cast<char*>(&size),sizeof(size),offset);
        buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
        try
        {
            this->channel_->ReadAsync(buf,offset + sizeof(size));
            batch.LoadFrom(buf);
            offset += size + sizeof(size);
        }
        catch(const std::exception&)
        {
            this->channel_->Truncate(offset);
            this->offset_ = offset;
            throw;   
        }
        batchs.emplace_back(std::move(batch));
    }
    return batchs;
}