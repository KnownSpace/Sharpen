#include <sharpen/PosixIoWriter.hpp>

#ifdef SHARPEN_IS_NIX

sharpen::PosixIoWriter::PosixIoWriter(sharpen::ErrorCode cancelErr)
    :Mybase()
    ,cancelErr_(cancelErr)
{}

sharpen::PosixIoWriter::~PosixIoWriter() noexcept
{
    this->CancelCallback();
    this->FillBufferAndCallback();
    this->CancelCallback();
}


void sharpen::PosixIoWriter::DoExecute(sharpen::FileHandle handle,bool &executed,bool &blocking)
{
    sharpen::Size size = this->GetRemainingSize();
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
    ssize_t bytes = ::writev(handle,bufs,size);
    if (bytes == -1)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (sharpen::IPosixIoOperator::IsBlockingError(err))
        {
            blocking = true;
            return;
        }
        for (size_t i = 0; i < size; i++)
        {
            cbs[i](-1);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    else if(bytes == 0)
    {
        for (size_t i = 0; i < size; i++)
        {
            cbs[i](0);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    sharpen::Size pending = this->ComputePendingSize();
    if (bytes == pending)
    {
        for (size_t i = 0; i < size; i++)
        {
            cbs[i](bufs[i].iov_len);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    sharpen::Size completed;
    sharpen::Size lastSize;
    this->ConvertByteToBufferNumber(bytes,completed,lastSize);
    for (size_t i = 0; i < completed; i++)
    {
        cbs[i](bufs[i].iov_len);
    }
    sharpen::Size lastBufSize = bufs[completed].iov_len;
    if (lastBufSize != lastSize)
    {
        sharpen::Uintptr p = reinterpret_cast<sharpen::Uintptr>(bufs[completed].iov_len);
        p += lastSize;
        bufs[completed].iov_base = reinterpret_cast<void*>(p);
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

void sharpen::PosixIoWriter::CancelCallback() noexcept
{
    Callback *cbs = this->GetFirstCallback();
    for (size_t i = 0,count = this->GetRemainingSize(); i < count; i++)
    {
        errno = this->cancelErr_;
        cbs[i](-1);
    }
}

#endif