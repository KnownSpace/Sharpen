#include <sharpen/IAsyncReadable.hpp>
#include <sharpen/AwaitableFuture.hpp>

sharpen::Size sharpen::IAsyncReadable::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->ReadAsync(buf,bufSize,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncReadable::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->ReadAsync(buf,bufferOffset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncReadable::ReadAsync(sharpen::ByteBuffer &buf)
{
    this->ReadAsync(buf,0);
}