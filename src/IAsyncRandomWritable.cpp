#include <sharpen/IAsyncRandomWritable.hpp>
#include <sharpen/AwaitableFuture.hpp>

std::size_t sharpen::IAsyncRandomWritable::WriteAsync(const char *buf,std::size_t bufSize,std::uint64_t offset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->WriteAsync(buf,bufSize,offset,future);
    return future.Await();
}

std::size_t sharpen::IAsyncRandomWritable::WriteAsync(const sharpen::ByteBuffer &buf,std::size_t bufferOffset,std::uint64_t offset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->WriteAsync(buf,bufferOffset,offset,future);
    return future.Await();
}

std::size_t sharpen::IAsyncRandomWritable::WriteAsync(const sharpen::ByteBuffer &buf,std::uint64_t offset)
{
    return this->WriteAsync(buf,0,offset);
}