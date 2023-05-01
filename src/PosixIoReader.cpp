#include <sharpen/PosixIoReader.hpp>

#ifdef SHARPEN_IS_NIX

void sharpen::PosixIoReader::NviExecute(sharpen::FileHandle handle,
                                        bool &executed,
                                        bool &blocking) {
    std::size_t size = this->GetRemainingSize();
    if (size == 0) {
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
        bytes = ::readv(handle, bufs, size);
    } while (bytes == -1 && sharpen::GetLastError() == EINTR);
    if (bytes == -1) {
        // blocking
        sharpen::ErrorCode err = sharpen::GetLastError();
        if (sharpen::IPosixIoOperator::IsBlockingError(err)) {
            blocking = true;
            return;
        }
        // error
        for (std::size_t i = 0; i != size; ++i) {
            cbs[i](-1);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    if (bytes == 0) {
        // disconnect
        for (std::size_t i = 0; i != size; ++i) {
            cbs[i](0);
        }
        size += this->GetMark();
        this->MoveMark(size);
        return;
    }
    // check completed buffer number
    std::size_t completed;
    std::size_t lastSize;
    this->ConvertByteToBufferNumber(bytes, completed, lastSize);
    // handle callback
    for (std::size_t i = 0; i != completed; ++i) {
        cbs[i](bufs[i].iov_len);
    }
    // last buffer
    std::size_t lastBufSize = bufs[completed].iov_len;
    cbs[completed](lastSize);
    completed += 1;
    completed += this->GetMark();
    this->MoveMark(completed);
    size = this->GetRemainingSize();
    if (lastBufSize != lastSize || size != 0) {
        blocking = true;
    }
}

#endif