#include <sharpen/IAsyncRandomWritable.hpp>
#include <sharpen/AwaitableFuture.hpp>

sharpen::Size sharpen::IAsyncRandomWritable::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Uint64 offset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->WriteAsync(buf,bufSize,offset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncRandomWritable::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Uint64 offset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->WriteAsync(buf,bufferOffset,offset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncRandomWritable::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Uint64 offset)
{
    return this->WriteAsync(buf,0,offset);
}