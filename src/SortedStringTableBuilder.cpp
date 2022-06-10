#include <sharpen/SortedStringTableBuilder.hpp>

sharpen::BloomFilter<sharpen::ByteBuffer> sharpen::SortedStringTableBuilder::LoadFilter(sharpen::FileChannelPtr channel, std::uint64_t offset, std::uint64_t size, std::size_t bitsOfElements)
{
    sharpen::ByteBuffer buf{sharpen::IntCast<std::size_t>(size)};
    channel->ReadAsync(buf, offset);
    sharpen::BloomFilter<sharpen::ByteBuffer> filter{buf.Data(), buf.GetSize(), bitsOfElements};
    return filter;
}

void sharpen::SortedStringTableBuilder::WriteFilters(sharpen::FileChannelPtr channel,const std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> &filters,sharpen::SstRoot &root,std::uint64_t &offset,sharpen::ByteBuffer &buf)
{
    for (std::size_t i = 0, count = filters.size(); i != count; ++i)
    {
        std::size_t size{filters[i].GetSize()};
        buf.ExtendTo(size);
        filters[i].CopyTo(buf.Data(),size);
        try
        {
            channel->WriteAsync(buf.Data(), size, offset);
        }
        catch (const std::exception &)
        {
            channel->Truncate();
            throw;
        }
        root.MetaIndexBlock()[i].Block().offset_ = offset;
        root.MetaIndexBlock()[i].Block().size_ = size;
        offset += size;
    }
}