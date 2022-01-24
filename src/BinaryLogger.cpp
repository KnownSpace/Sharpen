#include <sharpen/BinaryLogger.hpp>

#include <sharpen/FileOps.hpp>
#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::BinaryLogger::BinaryLogger(const char *logName,sharpen::EventEngine &engine)
    :channel_(nullptr)
    ,logName_(logName)
    ,offset_(0)
{
    this->channel_ = sharpen::MakeFileChannel(logName,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
    this->channel_->Register(engine);
    this->offset_ = this->channel_->GetFileSize();
}

sharpen::BinaryLogger::~BinaryLogger() noexcept
{
    if(this->channel_)
    {
        this->channel_->Close();
    }
}

sharpen::BinaryLogger &sharpen::BinaryLogger::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->channel_ = std::move(other.channel_);
        this->logName_ = std::move(other.logName_);
        this->offset_ = other.offset_;
        other.offset_ = 0;
    }
    return *this;
}

void sharpen::BinaryLogger::Remove()
{
    this->channel_->Close();
    sharpen::RemoveFile(this->logName_.c_str());   
}

void sharpen::BinaryLogger::Clear()
{
    this->channel_->Truncate();
}

void sharpen::BinaryLogger::Log(const sharpen::WriteBatch &batch)
{
    sharpen::ByteBuffer buf;
    batch.StoreTo(buf);
    sharpen::Uint64 size{buf.GetSize()};
    try
    {
        this->channel_->WriteAsync(reinterpret_cast<const char*>(&size),sizeof(size),this->offset_);
        this->channel_->WriteAsync(buf,this->offset_ + sizeof(size));
        this->offset_ += buf.GetSize() + sizeof(size);
    }
    catch(const std::exception&)
    {
        this->channel_->Truncate(this->offset_);
        throw;
    }
}

std::list<sharpen::WriteBatch> sharpen::BinaryLogger::GetWriteBatchs()
{
    std::list<sharpen::WriteBatch> batchs;
    sharpen::Uint64 offset{0};
    sharpen::ByteBuffer buf;
    while (offset != this->offset_)
    {
        sharpen::WriteBatch batch;
        sharpen::Uint64 size{0};
        this->channel_->ReadAsync(reinterpret_cast<char*>(&size),sizeof(size),offset);
        offset += sizeof(size);
        buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
        this->channel_->ReadAsync(buf,offset);
        offset += size;
        batch.LoadFrom(buf);
        batchs.emplace_back(std::move(batch));
    }
    return batchs;
}