#include <sharpen/IAsyncRandomReadable.hpp>

#include <sharpen/AwaitableFuture.hpp>

std::size_t sharpen::IAsyncRandomReadable::ReadAsync(char *buf,
                                                     std::size_t bufSize,
                                                     std::uint64_t offset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ReadAsync(buf, bufSize, offset, future);
    return future.Await();
}

std::size_t sharpen::IAsyncRandomReadable::ReadAsync(sharpen::ByteBuffer &buf,
                                                     std::size_t bufferOffset,
                                                     std::uint64_t offset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ReadAsync(buf, bufferOffset, offset, future);
    return future.Await();
}

std::size_t sharpen::IAsyncRandomReadable::ReadAsync(sharpen::ByteBuffer &buf, std::uint64_t offset)
{
    return this->ReadAsync(buf, 0, offset);
}