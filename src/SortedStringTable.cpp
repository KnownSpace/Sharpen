#include <sharpen/SortedStringTable.hpp>

#include <stdexcept>

#include <sharpen/IntOps.hpp>

void sharpen::SortedStringTable::Load(sharpen::FileChannelPtr channel)
{
    sharpen::ByteBuffer buf{sizeof(sharpen::SstFooter)};
    sharpen::Uint64 size = channel->GetFileSize();
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
        buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
        channel->ReadAsync(buf,this->footer_.IndexBlock().offset_);
        sharpen::Uint16 chksum{0};
        channel->ReadAsync(reinterpret_cast<char*>(&chksum),sizeof(chksum),this->footer_.IndexBlock().offset_ + size);
        if(chksum != buf.Crc16())
        {
            throw sharpen::ChecksumException("index block checksum error");
        }
        this->indexBlock_.LoadFrom(buf,buf.GetSize());
    }
    //load meta index block
    size = this->footer_.MetaIndexBlock().size_;
    if(size)
    {
        buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
        channel->ReadAsync(buf,this->footer_.MetaIndexBlock().offset_);
        sharpen::Uint16 chksum{0};
        channel->ReadAsync(reinterpret_cast<char*>(&chksum),sizeof(chksum),this->footer_.MetaIndexBlock().offset_ + size);
        if(chksum != buf.Crc16())
        {
            this->indexBlock_.Clear();
            throw sharpen::ChecksumException("meta index block checksum error");
        }
        this->metaIndexBlock_.LoadFrom(buf,buf.GetSize());
    }
}

void sharpen::SortedStringTable::Store(sharpen::FileChannelPtr channel,sharpen::Uint64 offset) const
{
    sharpen::ByteBuffer buf{4096};
    sharpen::Uint64 beginOffset = offset;
    sharpen::Uint64 metaIndexOffset = offset;
    //write meta index block
    sharpen::Size metaIndexSize = this->metaIndexBlock_.StoreTo(buf);
    try
    {
        if(metaIndexSize)
        {
            sharpen::Uint16 chksum{sharpen::Crc16(buf.Data(),metaIndexSize)};
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
    sharpen::Uint64 indexOffset = offset;
    sharpen::Size indexSize = this->indexBlock_.StoreTo(buf);
    try
    {
        if(indexSize)
        {
            sharpen::Uint16 chksum{sharpen::Crc16(buf.Data(),indexSize)};
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