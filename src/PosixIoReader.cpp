#include <sharpen/PosixIoReader.hpp>

#ifdef SHARPEN_IS_NIX

sharpen::PosixIoReader::PosixIoReader(sharpen::ErrorCode cancelErr)
    :Mybase()
    ,cancelErr_(cancelErr)
{}

sharpen::PosixIoReader::~PosixIoReader() noexcept
{
    this->CancelCallback();
    this->FillBufferAndCallback();
    this->CancelCallback();
}

void sharpen::PosixIoReader::DoExecute(sharpen::FileHandle handle,bool &blocking)
{
    sharpen::Size size = this->GetRemainingSize();
    if (!size)
    {
        blocking = false;
        return;
    }
    blocking = false;
    IoBuffer *bufs = this->GetFirstBuffer();
    Callback *cbs = this->GetFirstCallback();
    ssize_t bytes = ::readv(handle,bufs,size);
    if(bytes == -1)
    {
        sharpen::ErrorCode err = sharpen::GetLastError();
        if(sharpen::IPosixIoOperator::IsBlockingError(err))
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
    sharpen::Size number;
    sharpen::Size lastSize;
    this->ConvertByteToBufferNumber(bytes,number,lastSize);
    for (size_t i = 0; i < number; i++)
    {
        cbs[i](bufs[i].iov_len);
    }
    cbs[number](lastSize);
    number += 1;
    number += this->GetMark();
    this->MoveMark(number);
    if (lastSize < bufs[number].iov_len || this->GetRemainingSize())
    {
        blocking = true;
    }
}

void sharpen::PosixIoReader::CancelCallback() noexcept
{
    Callback *cbs = this->GetFirstCallback();
    for (size_t i = 0,count = this->GetRemainingSize(); i < count; i++)
    {
        errno = this->cancelErr_;
        cbs[i](-1);
    }
}

#endif