#include <sharpen/PosixIoWriter.hpp>

#ifdef SHARPEN_IS_NIX

void sharpen::PosixIoWriter::NviExecute(sharpen::FileHandle handle, bool &executed, bool &blocking)
{
    std::size_t size = this->GetRemainingSize();
    if (size == 0)
    {
        blocking = false;
        executed = false;
        return;
    }
    executed = true;
    blocking = false;
    IoBuffer *bufs = this->GetFirstBuffer();
    Callback *cbs = this->GetFirstCallback();
    ssize_t bytes;
    do {
        bytes = ::writev(handle, bufs, size);
    } while (bytes == -1 && sharpen::GetLastError() == EINTR);
    if (bytes == -1)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (sharpen::IPosixIoOperator::IsBlockingError(err))
        {
            blocking = true;
            return;
        }
        for (std::size_t i = 0; i != size; ++i)
        {
            cbs[i](-1);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    else if (bytes == 0)
    {
        for (std::size_t i = 0; i != size; ++i)
        {
            cbs[i](0);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    std::size_t completed;
    std::size_t lastSize;
    this->ConvertByteToBufferNumber(bytes, completed, lastSize);
    for (std::size_t i = 0; i != completed; ++i)
    {
        cbs[i](bufs[i].iov_len);
    }
    std::size_t lastBufSize = bufs[completed].iov_len;
    if (lastBufSize != lastSize)
    {
        std::uintptr_t p = reinterpret_cast<std::uintptr_t>(bufs[completed].iov_len);
        p += lastSize;
        bufs[completed].iov_base = reinterpret_cast<void *>(p);
        bufs[completed].iov_len -= lastSize;
    }
    else
    {
        cbs[completed](lastSize);
        completed += 1;
    }
    completed += this->GetMark();
    this->MoveMark(completed);
    size = this->GetRemainingSize();
    if (size != 0)
    {
        blocking = true;
    }
}

#endif