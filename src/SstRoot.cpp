#include <sharpen/SstRoot.hpp>

#include <stdexcept>

#include <sharpen/IntOps.hpp>

void sharpen::SstRoot::LoadFrom(sharpen::FileChannelPtr channel)
{
    sharpen::ByteBuffer buf{sizeof(sharpen::SstFooter)};
    std::uint64_t size = channel->GetFileSize();
    if(size < sizeof(sharpen::SstFooter))
    {
        throw std::invalid_argument("invalid sst file");
    }
    if(channel->ReadAsync(buf,size - sizeof(sharpen::SstFooter)) != sizeof(sharpen::SstFooter))
    {
        throw std::invalid_argument("invalid sst file");
    }
    //load footer
    this->footer_.LoadFrom(buf);
    //load index block
    size = this->footer_.IndexBlock().size_;
    if(size)
    {
        buf.ExtendTo(sharpen::IntCast<std::size_t>(size));
        channel->ReadAsync(buf,this->footer_.IndexBlock().offset_);
        std::uint16_t chksum{0};
        channel->ReadAsync(reinterpret_cast<char*>(&chksum),sizeof(chksum),this->footer_.IndexBlock().offset_ + size);
        if(chksum != buf.Crc16())
        {
            throw sharpen::DataCorruptionException("index block corruption");
        }
        this->indexBlock_.LoadFrom(buf);
    }
    //load meta index block
    size = this->footer_.MetaIndexBlock().size_;
    if(size)
    {
        buf.ExtendTo(sharpen::IntCast<std::size_t>(size));
        channel->ReadAsync(buf,this->footer_.MetaIndexBlock().offset_);
        std::uint16_t chksum{0};
        channel->ReadAsync(reinterpret_cast<char*>(&chksum),sizeof(chksum),this->footer_.MetaIndexBlock().offset_ + size);
        if(chksum != buf.Crc16())
        {
            this->indexBlock_.Clear();
            throw sharpen::DataCorruptionException("meta index block corruption");
        }
        this->metaIndexBlock_.LoadFrom(buf);
    }
}

void sharpen::SstRoot::StoreTo(sharpen::FileChannelPtr channel,std::uint64_t offset) const
{
    sharpen::ByteBuffer buf{4096};
    std::uint64_t beginOffset = offset;
    std::uint64_t metaIndexOffset = offset;
    //write meta index block
    std::size_t metaIndexSize = this->metaIndexBlock_.StoreTo(buf);
    try
    {
        if(metaIndexSize)
        {
            std::uint16_t chksum{sharpen::Crc16(buf.Data(),metaIndexSize)};
            offset += channel->WriteAsync(buf.Data(),metaIndexSize,offset);
            offset += channel->WriteAsync(reinterpret_cast<const char*>(&chksum),sizeof(chksum),offset);
        }
    }
    catch(const std::exception&)
    {
        channel->Truncate(beginOffset);
        throw;
    }
    //write index block
    std::uint64_t indexOffset = offset;
    std::size_t indexSize = this->indexBlock_.StoreTo(buf);
    try
    {
        if(indexSize)
        {
            std::uint16_t chksum{sharpen::Crc16(buf.Data(),indexSize)};
            offset += channel->WriteAsync(buf.Data(),indexSize,offset);
            offset += channel->WriteAsync(reinterpret_cast<const char*>(&chksum),sizeof(chksum),offset);
        }
    }
    catch(const std::exception&)
    {
        channel->Truncate(beginOffset);
        throw;
    }
    //write footer
    sharpen::SstFooter footer{this->footer_};
    footer.IndexBlock().offset_ = indexOffset;
    footer.MetaIndexBlock().offset_ = metaIndexOffset;
    footer.IndexBlock().size_ = indexSize;
    footer.MetaIndexBlock().size_ = metaIndexSize;
    try
    {
        offset += channel->WriteAsync(reinterpret_cast<char*>(&footer),sizeof(footer),offset);
        this->footer_ = std::move(footer);
    }
    catch(const std::exception&)
    {
        channel->Truncate(beginOffset);
        throw;
    }
}