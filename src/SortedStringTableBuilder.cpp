#include <sharpen/SortedStringTableBuilder.hpp>

sharpen::BloomFilter<sharpen::ByteBuffer> sharpen::SortedStringTableBuilder::BuildFilter(sharpen::FileChannelPtr channel,sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::Size bitsOfElements)
{
    sharpen::ByteBuffer buf{sharpen::IntCast<sharpen::Size>(size)};
    channel->ReadAsync(buf,offset);
    sharpen::BloomFilter<sharpen::ByteBuffer> filter{buf.Data(),buf.GetSize(),bitsOfElements};
    return filter;
}