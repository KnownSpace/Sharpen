#include <sharpen/IAsyncReadable.hpp>

#include <sharpen/AwaitableFuture.hpp>

std::size_t sharpen::IAsyncReadable::ReadAsync(char *buf, std::size_t bufSize)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ReadAsync(buf, bufSize, future);
    return future.Await();
}

std::size_t sharpen::IAsyncReadable::ReadAsync(sharpen::ByteBuffer &buf, std::size_t bufferOffset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ReadAsync(buf, bufferOffset, future);
    return future.Await();
}

std::size_t sharpen::IAsyncReadable::ReadAsync(sharpen::ByteBuffer &buf)
{
    return this->ReadAsync(buf, 0);
}