#include <sharpen/WalStatusMap.hpp>

#include <sharpen/BufferReader.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IEventLoopGroup.hpp>
#include <sharpen/IntOps.hpp>

sharpen::WalStatusMap::WalStatusMap(std::string name)
    : WalStatusMap(sharpen::GetLocalLoopGroup(), std::move(name)) {
}

sharpen::WalStatusMap::WalStatusMap(sharpen::IEventLoopGroup &loopGroup, std::string name)
    : name_(std::move(name))
    , tempName_()
    , channel_(nullptr)
    , loopGroup_(&loopGroup)
    , map_()
    , lock_(nullptr)
    , offset_(0)
    , contentSize_(0) {
    assert(!this->name_.empty());
    this->tempName_.resize(this->name_.size() + 4);
    std::memcpy(const_cast<char *>(this->tempName_.data()), this->name_.data(), this->name_.size());
    std::memcpy(const_cast<char *>(this->tempName_.data() + this->name_.size()), ".tmp", 4);
    assert(!this->tempName_.empty());
    sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(this->name_.c_str(),
                                                             sharpen::FileAccessMethod::All,
                                                             sharpen::FileOpenMethod::CreateOrOpen,
                                                             sharpen::FileIoMethod::Sync)};
    channel->Register(*this->loopGroup_);
    this->channel_ = std::move(channel);
    this->lock_.reset(new (std::nothrow) sharpen::AsyncRwLock{});
    if (!this->lock_) {
        throw std::bad_alloc{};
    }
    this->Load();
    this->contentSize_ = this->ComputeContentSize();
}

bool sharpen::WalStatusMap::Insert(sharpen::ByteBuffer key, sharpen::ByteBuffer value) {
    auto ite = this->map_.find(key);
    if (ite != this->map_.end()) {
        if (ite->second == value) {
            return false;
        }
        this->contentSize_ -= ite->second.GetSize();
        this->contentSize_ += value.GetSize();
        ite->second = std::move(value);
    } else {
        this->contentSize_ += key.GetSize();
        this->contentSize_ += value.GetSize();
        this->map_.emplace(std::move(key), std::move(value));
    }
    return true;
}

bool sharpen::WalStatusMap::Erase(const sharpen::ByteBuffer &key) noexcept {
    auto ite = this->map_.find(key);
    if (ite != this->map_.end()) {
        this->contentSize_ -= ite->first.GetSize();
        this->contentSize_ -= ite->second.GetSize();
        this->map_.erase(ite);
        return true;
    }
    return false;
}

std::size_t sharpen::WalStatusMap::ComputeContentSize() const noexcept {
    std::size_t size{0};
    for (auto begin = this->map_.begin(), end = this->map_.end(); begin != end; ++begin) {
        size += begin->first.GetSize() + begin->second.GetSize();
    }
    return size;
}

void sharpen::WalStatusMap::Load() {
    std::uint64_t size{this->channel_->GetFileSize()};
    if (size) {
        sharpen::ByteBuffer buf{sharpen::IntCast<std::size_t>(size)};
        std::size_t sz{this->channel_->ReadAsync(buf, 0)};
        assert(sz == size);
        (void)sz;
        sharpen::BufferReader reader{buf};
        while (reader.GetOffset() != size) {
            std::size_t offset{reader.GetOffset()};
            std::uint8_t tag{0};
            reader.Read(tag);
            switch (tag) {
            case writeTag_: {
                sharpen::ByteBuffer key;
                sharpen::ByteBuffer value;
                try {
                    reader.Read(key);
                    reader.Read(value);
                } catch (const std::out_of_range &error) {
                    this->channel_->Truncate(offset);
                    this->offset_ = offset;
                    (void)error;
                    return;
                }
                bool result{this->Insert(std::move(key), std::move(value))};
                assert(result);
                (void)result;
            } break;
            case removeTag_: {
                sharpen::ByteBuffer key;
                try {
                    reader.Read(key);
                } catch (const std::out_of_range &error) {
                    this->channel_->Truncate(offset);
                    this->offset_ = offset;
                    (void)error;
                    return;
                }
                bool result{this->Erase(key)};
                assert(result);
                (void)result;
            } break;
            default:
                this->channel_->Truncate(offset);
                break;
            }
        }
    }
    this->offset_ = size;
}

void sharpen::WalStatusMap::RebuildFile() {
    assert(this->loopGroup_);
    assert(!this->name_.empty());
    assert(!this->tempName_.empty());
    sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(this->tempName_.c_str(),
                                                             sharpen::FileAccessMethod::Write,
                                                             sharpen::FileOpenMethod::CreateNew)};
    std::uint64_t offset{0};
    channel->Register(*this->loopGroup_);
    if (!this->map_.empty()) {
        sharpen::ByteBuffer buf{4096};
        for (auto begin = this->map_.begin(), end = this->map_.end(); begin != end; ++begin) {
            sharpen::BufferWriter writer{buf};
            std::uint8_t tag{writeTag_};
            writer.Write(tag);
            writer.Write(begin->first);
            writer.Write(begin->second);
            std::size_t sz{channel->WriteAsync(buf.Data(), writer.GetLength(), offset)};
            assert(sz == writer.GetLength());
            if (sz != writer.GetLength()) {
                sharpen::ThrowSystemError(sharpen::ErrorIo);
            }
            offset += sz;
        }
    }
    channel->FlushAsync();
    channel->Close();
    this->channel_->Close();
    sharpen::RenameFile(this->tempName_.c_str(), this->name_.c_str());
    this->offset_ = offset;
    this->channel_ = sharpen::OpenFileChannel(this->name_.c_str(),
                                              sharpen::FileAccessMethod::Write,
                                              sharpen::FileOpenMethod::CreateOrOpen);
    this->channel_->Register(*this->loopGroup_);
    this->contentSize_ = this->ComputeContentSize();
}

void sharpen::WalStatusMap::NviWrite(sharpen::ByteBuffer key, sharpen::ByteBuffer value) {
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        bool result{this->Insert(key, value)};
        if (result) {
            std::size_t contentSize{this->contentSize_};
            if (contentSize && this->offset_ >= contentSize * limitFactor_) {
                this->RebuildFile();
                return;
            }
            std::size_t pairSize{key.ComputeSize() + value.ComputeSize() + sizeof(std::uint8_t)};
            std::uint8_t tag{writeTag_};
            sharpen::ByteBuffer buf{pairSize};
            sharpen::BufferWriter writer{buf};
            writer.Write(tag);
            writer.Write(key);
            writer.Write(value);
            std::size_t sz{this->channel_->WriteAsync(buf, this->offset_)};
            assert(sz == buf.GetSize());
            if (sz != buf.GetSize()) {
                this->channel_->Truncate(this->offset_);
                sharpen::ThrowSystemError(sharpen::ErrorIo);
            }
            this->channel_->FlushAsync();
            this->offset_ += sz;
        }
    }
}

void sharpen::WalStatusMap::NviRemove(const sharpen::ByteBuffer &key) {
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        bool result{this->Erase(key)};
        if (result) {
            std::size_t contentSize{this->contentSize_};
            if (this->offset_ >= contentSize * limitFactor_) {
                this->RebuildFile();
                return;
            }
            std::size_t pairSize{key.ComputeSize() + sizeof(std::uint8_t)};
            std::uint8_t tag{removeTag_};
            sharpen::ByteBuffer buf{pairSize};
            sharpen::BufferWriter writer{buf};
            writer.Write(tag);
            writer.Write(key);
            std::size_t sz{this->channel_->WriteAsync(buf, this->offset_)};
            assert(sz == buf.GetSize());
            if (sz != buf.GetSize()) {
                this->channel_->Truncate(this->offset_);
                sharpen::ThrowSystemError(sharpen::ErrorIo);
            }
            this->channel_->FlushAsync();
            this->offset_ += sz;
        }
    }
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::WalStatusMap::NviLookup(
    const sharpen::ByteBuffer &key) const {
    {
        this->lock_->LockRead();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        auto ite = this->map_.find(key);
        if (ite != this->map_.end()) {
            return ite->second;
        }
        return sharpen::EmptyOpt;
    }
}

sharpen::WalStatusMap::WalStatusMap(Self &&other) noexcept
    : name_(std::move(other.name_))
    , tempName_(std::move(other.tempName_))
    , channel_(std::move(other.channel_))
    , loopGroup_(other.loopGroup_)
    , map_(std::move(other.map_))
    , lock_(std::move(other.lock_))
    , offset_(other.offset_)
    , contentSize_(other.contentSize_) {
    other.loopGroup_ = nullptr;
    other.offset_ = 0;
    other.contentSize_ = 0;
}

sharpen::WalStatusMap &sharpen::WalStatusMap::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->name_ = std::move(other.name_);
        this->tempName_ = std::move(other.tempName_);
        this->loopGroup_ = other.loopGroup_;
        this->map_ = std::move(other.map_);
        this->lock_ = std::move(other.lock_);
        this->offset_ = other.offset_;
        this->contentSize_ = other.contentSize_;
        other.loopGroup_ = nullptr;
        other.offset_ = 0;
        other.contentSize_ = 0;
    }
    return *this;
}