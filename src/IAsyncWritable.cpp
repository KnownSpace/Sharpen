#include <sharpen/IAsyncWritable.hpp>
#include <sharpen/AwaitableFuture.hpp>

sharpen::Size sharpen::IAsyncWritable::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->WriteAsync(buf,bufSize,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncWritable::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset)
{
    sharpen::AwaitableFuture<sharpen::Size> future;
    this->WriteAsync(buf,bufferOffset,future);
    return future.Await();
}

sharpen::Size sharpen::IAsyncWritable::WriteAsync(const sharpen::ByteBuffer &buf)
{
    return this->WriteAsync(buf,0);
}