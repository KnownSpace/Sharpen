#include <sharpen/BinarySerializable.hpp>
#include <sharpen/BufferReader.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/ByteBuffer.hpp>
#include <sharpen/EventLoop.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IntOps.hpp>
#include <sharpen/IteratorOps.hpp>
#include <sharpen/LogEntries.hpp>
#include <sharpen/Varint.hpp>
#include <sharpen/WalLogStorage.hpp>
#include <cstdint>

sharpen::WalLogStorage::WalLogStorage(std::string name)
    : Self{sharpen::GetLocalLoopGroup(), std::move(name)} {
}

sharpen::WalLogStorage::WalLogStorage(sharpen::IEventLoopGroup &loopGroup, std::string name)
    : name_(std::move(name))
    , tempName_()
    , channel_(nullptr)
    , loopGroup_(&loopGroup)
    , lock_(nullptr)
    , logs_()
    , offset_(0)
    , contentSize_(0) {
    assert(!this->name_.empty());
    sharpen::AsyncRwLock *lock{new (std::nothrow) sharpen::AsyncRwLock{}};
    if (!lock) {
        throw std::bad_alloc{};
    }
    this->lock_.reset(lock);
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
    this->Load();
    this->contentSize_ = this->ComputeContentSize();
}

bool sharpen::WalLogStorage::Insert(std::uint64_t index, sharpen::ByteBuffer log) {
    auto ite = this->logs_.find(index);
    if (ite != this->logs_.end()) {
        if (ite->second == log) {
            return false;
        }
        this->contentSize_ -= ite->second.GetSize();
        this->contentSize_ += log.GetSize();
        ite->second = std::move(log);
    } else {
        sharpen::Varuint64 builder{index};
        this->contentSize_ += builder.ComputeSize();
        this->contentSize_ += log.GetSize();
        this->logs_.emplace(index, std::move(log));
    }
    return true;
}

bool sharpen::WalLogStorage::Erase(std::uint64_t index) noexcept {
    auto ite = this->logs_.find(index);
    if (ite != this->logs_.end()) {
        sharpen::Varuint64 builder{ite->first};
        this->contentSize_ -= builder.ComputeSize();
        this->contentSize_ -= ite->second.GetSize();
        this->logs_.erase(ite);
        return true;
    }
    return false;
}

void sharpen::WalLogStorage::Load() {
    std::uint64_t size{this->channel_->GetFileSize()};
    if (size) {
        sharpen::ByteBuffer buf{sharpen::IntCast<std::size_t>(size)};
        std::size_t sz{this->channel_->ReadAsync(buf, 0)};
        assert(sz == size);
        (void)sz;
        sharpen::BufferReader reader{buf};
        while (reader.GetOffset() != buf.GetSize()) {
            std::size_t offset{reader.GetOffset()};
            std::uint8_t tag{0};
            reader.Read(tag);
            std::uint64_t index{0};
            switch (tag) {
            case Self::writeTag_: {
                sharpen::Varuint64 builder{0};
                sharpen::ByteBuffer log;
                try {
                    reader.Read(builder);
                    reader.Read(log);
                } catch (const std::out_of_range &error) {
                    (void)error;
                    this->channel_->Truncate(offset);
                    this->offset_ = offset;
                    return;
                }
                index = builder.Get();
                bool result{this->Insert(index, std::move(log))};
                assert(result);
                (void)result;
            } break;
            case Self::removeTag_: {
                sharpen::Varuint64 builder{0};
                try {
                    reader.Read(builder);
                } catch (const std::out_of_range &error) {
                    (void)error;
                    this->channel_->Truncate(offset);
                    this->offset_ = offset;
                    return;
                }
                index = builder.Get();
                bool result{this->Erase(index)};
                assert(result);
                (void)result;
            } break;
            default:
                this->channel_->Truncate(offset);
                break;
            }
        }
        this->offset_ = size;
    }
}

std::size_t sharpen::WalLogStorage::ComputeContentSize() const noexcept {
    std::size_t size{0};
    for (auto begin = this->logs_.begin(), end = this->logs_.end(); begin != end; ++begin) {
        sharpen::Varuint64 builder{begin->first};
        size += builder.ComputeSize();
        size += begin->second.ComputeSize();
    }
    return size;
}

void sharpen::WalLogStorage::RebuildFile() {
    assert(this->loopGroup_);
    assert(!this->name_.empty());
    assert(!this->tempName_.empty());
    sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(this->tempName_.c_str(),
                                                             sharpen::FileAccessMethod::Write,
                                                             sharpen::FileOpenMethod::CreateNew)};
    std::uint64_t offset{0};
    channel->Register(*this->loopGroup_);
    if (!this->logs_.empty()) {
        sharpen::ByteBuffer buf{4096};
        for (auto begin = this->logs_.begin(), end = this->logs_.end(); begin != end; ++begin) {
            sharpen::BufferWriter writer{buf};
            std::uint8_t tag{writeTag_};
            writer.Write(tag);
            sharpen::Varuint64 builder{begin->first};
            writer.Write(builder);
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

sharpen::Optional<sharpen::ByteBuffer> sharpen::WalLogStorage::NviLookup(
    std::uint64_t index) const {
    {
        this->lock_->LockRead();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        auto ite = this->logs_.find(index);
        if (ite != this->logs_.end()) {
            return sharpen::EmptyOpt;
        }
        return ite->second;
    }
}

void sharpen::WalLogStorage::NviWrite(std::uint64_t index, sharpen::ByteSlice log) {
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        sharpen::ByteBuffer buf{log};
        bool result{this->Insert(index, std::move(buf))};
        if (result) {
            std::size_t contentSize{this->contentSize_};
            if (contentSize && this->offset_ >= contentSize * limitFactor_) {
                this->RebuildFile();
                return;
            }
            sharpen::Varuint64 builder{index};
            std::size_t pairSize{builder.ComputeSize() + log.ComputeSize() + sizeof(std::uint8_t)};
            std::uint8_t tag{writeTag_};
            buf = sharpen::ByteBuffer{pairSize};
            sharpen::BufferWriter writer{buf};
            writer.Write(tag);
            writer.Write(builder);
            writer.Write(log);
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

void sharpen::WalLogStorage::NviWriteBatch(std::uint64_t beginIndex, sharpen::LogEntries entries) {
    {
        std::size_t size{0};
        for (std::size_t i = 0; i != entries.GetSize(); ++i) {
            std::size_t sz{sizeof(std::uint8_t)};
            std::uint64_t index{i + beginIndex};
            sharpen::Varuint64 builder{index};
            sz += sharpen::BinarySerializator::ComputeSize(builder);
            sz += sharpen::BinarySerializator::ComputeSize(entries.Get(i));
            size += sz;
        }
        if (size) {
            this->lock_->LockWrite();
            std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
            sharpen::ByteBuffer buf{size};
            sharpen::BufferWriter writer{buf};
            for (std::size_t i = 0; i != entries.GetSize(); ++i) {
                std::uint64_t index{i + beginIndex};
                if (this->Insert(index, entries.Get(i))) {
                    writer.Write(writeTag_);
                    sharpen::Varuint64 builder{index};
                    writer.Write(builder);
                    writer.Write(entries.Get(i));
                }
            }
            size = writer.GetLength();
            if (size) {
                std::size_t sz{this->channel_->WriteAsync(buf, this->offset_)};
                assert(sz == size);
                if (sz != size) {
                    this->channel_->Truncate(this->offset_);
                    sharpen::ThrowSystemError(sharpen::ErrorIo);
                }
                this->channel_->FlushAsync();
                this->offset_ += sz;
            }
        }
    }
}

void sharpen::WalLogStorage::NviDropUntil(std::uint64_t index) noexcept {
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        std::size_t size{0};
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first < index;
             ++begin) {
            sharpen::Varuint64 builder{begin->first};
            size += builder.ComputeSize();
            size += begin->second.GetSize();
        }
        if (!size) {
            return;
        }
        std::size_t contentSize{this->contentSize_};
        contentSize -= size;
        if (this->offset_ >= contentSize * limitFactor_) {
            for (auto begin = this->logs_.begin(), end = this->logs_.end();
                 begin != end && begin->first < index;
                 ++begin) {
                begin = this->logs_.erase(begin);
            }
            this->RebuildFile();
            return;
        }
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first < index;
             ++begin) {
            size += sizeof(std::uint8_t);
            size -= begin->second.GetSize();
        }
        sharpen::ByteBuffer buf{size};
        sharpen::BufferWriter writer{buf};
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first < index;
             ++begin) {
            writer.Write(removeTag_);
            sharpen::Varuint64 builder{begin->first};
            writer.Write(builder);
        }
        std::size_t sz{this->channel_->WriteAsync(buf, this->offset_)};
        assert(sz == buf.GetSize());
        if (sz != buf.GetSize()) {
            this->channel_->Truncate(this->offset_);
            return;
        }
        this->channel_->FlushAsync();
        this->contentSize_ = contentSize;
        this->offset_ += sz;
    }
}

void sharpen::WalLogStorage::NviTruncateFrom(std::uint64_t index) {
    {
        this->lock_->LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        std::size_t size{0};
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first >= index;
             ++begin) {
            sharpen::Varuint64 builder{begin->first};
            size += builder.ComputeSize();
            size += begin->second.GetSize();
        }
        if (!size) {
            return;
        }
        std::size_t contentSize{this->contentSize_};
        contentSize -= size;
        if (this->offset_ >= contentSize * limitFactor_) {
            for (auto begin = this->logs_.begin(), end = this->logs_.end();
                 begin != end && begin->first >= index;
                 ++begin) {
                begin = this->logs_.erase(begin);
            }
            this->RebuildFile();
            return;
        }
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first >= index;
             ++begin) {
            size += sizeof(std::uint8_t);
            size -= begin->second.GetSize();
        }
        sharpen::ByteBuffer buf{size};
        sharpen::BufferWriter writer{buf};
        for (auto begin = this->logs_.begin(), end = this->logs_.end();
             begin != end && begin->first >= index;
             ++begin) {
            writer.Write(removeTag_);
            sharpen::Varuint64 builder{begin->first};
            writer.Write(builder);
        }
        std::size_t sz{this->channel_->WriteAsync(buf, this->offset_)};
        assert(sz == buf.GetSize());
        if (sz != buf.GetSize()) {
            this->channel_->Truncate(this->offset_);
            sharpen::ThrowSystemError(sharpen::ErrorIo);
        }
        this->channel_->FlushAsync();
        this->contentSize_ = contentSize;
        this->offset_ += sz;
    }
}

std::uint64_t sharpen::WalLogStorage::GetLastIndex() const {
    {
        this->lock_->LockRead();
        std::unique_lock<sharpen::AsyncRwLock> lock{*this->lock_, std::adopt_lock};
        std::uint64_t index{0};
        if (!this->logs_.empty()) {
            return this->logs_.rbegin()->first;
        }
        return index;
    }
}

sharpen::WalLogStorage::WalLogStorage(Self &&other) noexcept
    : name_(std::move(other.name_))
    , tempName_(std::move(other.tempName_))
    , channel_(std::move(other.channel_))
    , loopGroup_(other.loopGroup_)
    , lock_(std::move(other.lock_))
    , logs_(std::move(other.logs_))
    , offset_(other.offset_)
    , contentSize_(other.contentSize_) {
    other.loopGroup_ = nullptr;
    other.offset_ = 0;
    other.contentSize_ = 0;
}

sharpen::WalLogStorage &sharpen::WalLogStorage::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->name_ = std::move(other.name_);
        this->tempName_ = std::move(other.tempName_);
        this->channel_ = std::move(other.channel_);
        this->loopGroup_ = other.loopGroup_;
        this->lock_ = std::move(other.lock_);
        this->logs_ = std::move(other.logs_);
        this->offset_ = other.offset_;
        this->contentSize_ = other.contentSize_;
        other.loopGroup_ = nullptr;
        other.offset_ = 0;
        other.contentSize_ = 0;
    }
    return *this;
}