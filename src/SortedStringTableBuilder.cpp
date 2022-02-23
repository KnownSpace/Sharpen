#include <sharpen/SortedStringTableBuilder.hpp>

sharpen::BloomFilter<sharpen::ByteBuffer> sharpen::SortedStringTableBuilder::LoadFilter(sharpen::FileChannelPtr channel, sharpen::Uint64 offset, sharpen::Uint64 size, sharpen::Size bitsOfElements)
{
    sharpen::ByteBuffer buf{sharpen::IntCast<sharpen::Size>(size)};
    channel->ReadAsync(buf, offset);
    sharpen::BloomFilter<sharpen::ByteBuffer> filter{buf.Data(), buf.GetSize(), bitsOfElements};
    return filter;
}

void sharpen::SortedStringTableBuilder::WriteFilters(sharpen::FileChannelPtr channel,const std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> &filters,sharpen::SstRoot &root,sharpen::Uint64 &offset,sharpen::ByteBuffer &buf)
{
    for (sharpen::Size i = 0, count = filters.size(); i != count; ++i)
    {
        sharpen::Size size{filters[i].GetSize()};
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