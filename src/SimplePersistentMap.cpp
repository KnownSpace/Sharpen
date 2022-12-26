#include <sharpen/SimplePersistentMap.hpp>

#include <sharpen/SystemError.hpp>
#include <sharpen/BufferWriter.hpp>

sharpen::SimplePersistentMap::SimplePersistentMap(sharpen::FileChannelPtr file)
    :file_(std::move(file))
    ,map_()
    ,lock_()
{
    assert(this->file_);
    this->ReadFromFile(0);
}

void sharpen::SimplePersistentMap::SaveFile()
{
    assert(this->file_);
    std::uint64_t count{this->map_.size()};
    if(count)
    {
        std::uint64_t totalSize{sizeof(count)};
        for(auto begin = this->map_.begin(),end = this->map_.end(); begin != end; ++begin)
        {
            totalSize += 2*sizeof(std::uint64_t);
            totalSize += begin->first.GetSize();
            totalSize += begin->second.GetSize();
        }
        sharpen::ByteBuffer buf{totalSize};
        sharpen::BufferWriter writer{buf};
        writer.Write(count);
        for(auto begin = this->map_.begin(),end = this->map_.end(); begin != end; ++begin)
        {
            writer.Write(begin->first.GetSize());
            writer.Write(begin->second.GetSize());
            writer.Write(begin->first.Data(),begin->first.GetSize());
            writer.Write(begin->second.Data(),begin->second.GetSize());
        }
        this->file_->WriteAsync(buf,0);
    }
}

void sharpen::SimplePersistentMap::ReadFromFile(std::uint64_t offset)
{
    assert(this->file_);
    std::uint64_t fileSize{this->file_->GetFileSize() - offset};
    if(fileSize >= sizeof(std::uint64_t))
    {
        std::uint64_t count{0};
        offset += this->file_->ReadObjectAsync(count,offset);
        this->map_.clear();
        while (this->map_.size() != count && fileSize != offset + 2*sizeof(std::uint64_t))
        {
            std::uint64_t sizes[2] = {0,0};
            offset += this->file_->ReadAsync(reinterpret_cast<char*>(sizes),sizeof(sizes),offset);
            std::uint64_t keySize{sizes[0]};
            std::uint64_t valueSize{sizes[0]};
            sharpen::ByteBuffer key{keySize};
            sharpen::ByteBuffer value{valueSize};
            offset += this->file_->ReadAsync(key,offset);
            offset += this->file_->ReadAsync(value,offset);
            this->map_.emplace(std::move(key),std::move(value));
        }
    }
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::SimplePersistentMap::NviLookup(const sharpen::ByteBuffer &key) const
{
    {
        this->lock_.LockRead();
        std::unique_lock<sharpen::AsyncRwLock> lock{this->lock_,std::adopt_lock};
        auto ite = this->map_.find(key);
        if(ite != this->map_.end())
        {
            return ite->second;
        }
        return sharpen::EmptyOpt;
    }
}

void sharpen::SimplePersistentMap::NviWrite(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    {
        this->lock_.LockWrite();
        std::unique_lock<sharpen::AsyncRwLock> lock{this->lock_,std::adopt_lock};
        auto ite = this->map_.find(key);
        bool motifyed{false};
        if(ite != this->map_.end())
        {
            if(ite->second != value)
            {
                motifyed = true;
                ite->second = value;
            }
        }
        else
        {
            motifyed = true;
            this->map_.emplace(std::move(key),std::move(value));
        }
        if(motifyed)
        {
            this->SaveFile();
        }
    }
}