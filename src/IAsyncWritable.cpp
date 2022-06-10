#include <sharpen/IAsyncWritable.hpp>
#include <sharpen/AwaitableFuture.hpp>

std::size_t sharpen::IAsyncWritable::WriteAsync(const char *buf,std::size_t bufSize)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->WriteAsync(buf,bufSize,future);
    return future.Await();
}

std::size_t sharpen::IAsyncWritable::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->WriteAsync(buf,bufferOffset,future);
    return future.Await();
}

std::size_t sharpen::IAsyncWritable::WriteAsync(const sharpen::ByteBuffer &buf)
{
    return this->WriteAsync(buf,0);
}