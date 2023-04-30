#include <sharpen/CowStatusMap.hpp>

#include <sharpen/BufferReader.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IntOps.hpp>

sharpen::CowStatusMap::CowStatusMap(std::string name)
    : CowStatusMap(sharpen::GetLocalLoopGroup(), std::move(name))
{
}

void sharpen::CowStatusMap::Load()
{
    sharpen::FileChannelPtr channel{
        sharpen::OpenFileChannel(this->name_.c_str(),
                                 sharpen::FileAccessMethod::Read,
                                 sharpen::FileOpenMethod::CreateOrOpen)};
    channel->Register(*this->loopGroup_);
    std::uint64_t size{channel->GetFileSize()};
    sharpen::ByteBuffer buf{sharpen::IntCast<std::size_t>(size)};
    std::size_t sz{channel->ReadAsync(buf, 0)};
    assert(sz == size);
    (void)sz;
    sharpen::BufferReader reader{buf};
    while (reader.GetOffset() != buf.GetSize())
    {
        sharpen::ByteBuffer key;
        sharpen::ByteBuffer value;
        reader.Read(key);
        reader.Read(value);
        this->map_.emplace(std::move(key), std::move(value));
    }
}

sharpen::CowStatusMap::CowStatusMap(sharpen::IEventLoopGroup &loopGroup, std::string name)
    : name_(std::move(name))
    , tempName_()
    , map_()
    , loopGroup_(&loopGroup)
    , lock_(nullptr)
{
    assert(!this->name_.empty());
    this->tempName_.resize(this->name_.size() + 4);
    std::memcpy(const_cast<char *>(this->tempName_.data()), this->name_.data(), this->name_.size());
    std::memcpy(const_cast<char *>(this->tempName_.data() + this->name_.size()), ".tmp", 4);
    this->lock_.reset(new (std::nothrow) sharpen::AsyncRwLock{});
    if (!this->lock_)
    {
        throw std::bad_alloc{};
    }
    this->Load();
}

void sharpen::CowStatusMap::Save()
{
    assert(!this->tempName_.empty());
    assert(!this->name_.empty());
    if (!this->map_.empty())
    {
        sharpen::FileChannelPtr channel{
            sharpen::OpenFileChannel(this->tempName_.c_str(),
                                     sharpen::FileAccessMethod::Write,
                                     sharpen::FileOpenMethod::CreateNew)};
        channel->Register(*this->loopGroup_);
        std::uint64_t offset{0};
        sharpen::ByteBuffer buf{4096};
        for (auto begin = this->map_.begin(), end = this->map_.end(); begin != end; ++begin)
        {
            sharpen::BufferWriter writer{buf};
            writer.Write(begin->first);
            writer.Write(begin->second);
            std::size_t sz{channel->WriteAsync(buf.Data(), writer.GetLength(), offset)};
            assert(sz == writer.GetLength());
            if (sz != writer.GetLength())
            {
                sharpen::ThrowSystemError(sharpen::ErrorIo);
            }
            offset += sz;
        }
        channel->FlushAsync();
        channel->Close();
    }
    else
    {
        // trunc the file
        sharpen::FileChannelPtr channel{
            sharpen::OpenFileChannel(this->tempName_.c_str(),
                                     sharpen::FileAccessMethod::Write,
                                     sharpen::FileOpenMethod::CreateNew)};
        channel->Register(*this->loopGroup_);
        channel->FlushAsync();
        channel->Close();
    }
    sharpen::RenameFile(this->tempName_.c_str(), this->name_.c_str());
}

void sharpen::CowStatusMap::NviRemove(const sharpen::ByteBuffer &key)
{
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        auto ite = this->map_.find(key);
        if (ite != this->map_.end())
        {
            this->map_.erase(ite);
            this->Save();
        }
    }
}

void sharpen::CowStatusMap::NviWrite(sharpen::ByteBuffer key, sharpen::ByteBuffer value)
{
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        auto ite = this->map_.find(key);
        if (ite != this->map_.end())
        {
            if (ite->second == value)
            {
                return;
            }
            ite->second = std::move(value);
        }
        else
        {
            this->map_.emplace(std::move(key), std::move(value));
        }
        this->Save();
    }
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::CowStatusMap::NviLookup(
    const sharpen::ByteBuffer &key) const
{
    {
        this->lock_->LockRead();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        auto ite = this->map_.find(key);
        if (ite != this->map_.end())
        {
            return ite->second;
        }
        return sharpen::EmptyOpt;
    }
}

sharpen::CowStatusMap::CowStatusMap(Self &&other) noexcept
    : name_(std::move(other.name_))
    , tempName_(std::move(other.tempName_))
    , map_(std::move(other.map_))
    , loopGroup_(other.loopGroup_)
    , lock_(std::move(other.lock_))
{
    other.loopGroup_ = nullptr;
}

sharpen::CowStatusMap &sharpen::CowStatusMap::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->name_ = std::move(other.name_);
        this->tempName_ = std::move(other.tempName_);
        this->map_ = std::move(other.map_);
        this->loopGroup_ = other.loopGroup_;
        this->lock_ = std::move(other.lock_);
        other.loopGroup_ = nullptr;
    }
    return *this;
}