#include <sharpen/IAsyncRandomReadable.hpp>
#include <sharpen/AwaitableFuture.hpp>

sharpen::Size sharpen::IAsyncRandomReadable::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->ReadAsync(buf,bufSize,offset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncRandomReadable::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->ReadAsync(buf,bufferOffset,offset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncRandomReadable::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Uint64 offset)
{
    return this->ReadAsync(buf,0,offset);
}