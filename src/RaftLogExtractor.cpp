#include <sharpen/RaftLogExtractor.hpp>

#include <sharpen/Varint.hpp>

std::uint64_t sharpen::RaftLogExtractor::NviExtractTerm(sharpen::ByteSlice log) const
{
    sharpen::Varuint64 builder{log.Data(),(std::min)(log.GetSize(),sharpen::Varuint64::GetMaxSize())};
    return builder.Get();
}