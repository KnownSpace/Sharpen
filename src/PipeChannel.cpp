#include <sharpen/PipeChannel.hpp>

#include <sharpen/PipeChannelImpl.hpp>

#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#include <sharpen/WinEx.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

void sharpen::OpenPipeChannel(sharpen::InputPipeChannelPtr &in, sharpen::OutputPipeChannelPtr &out)
{
    sharpen::FileHandle inHandle;
    sharpen::FileHandle outHandle;
#ifdef SHARPEN_HAS_WININPUTPIPE
    BOOL r = ::CreatePipeEx(
        &inHandle, &outHandle, nullptr, 0, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED);
    if (r == FALSE)
    {
        sharpen::ThrowLastError();
    }
#else
    sharpen::FileHandle fd[2];
    int r = ::pipe2(fd, O_NONBLOCK | O_CLOEXEC);
    if (r == -1)
    {
        sharpen::ThrowLastError();
    }
    inHandle = fd[0];
    outHandle = fd[1];
#endif
    try
    {
        in = std::make_shared<sharpen::InputPipeChannelImpl>(inHandle);
    }
    catch (const std::exception &rethrow)
    {
        sharpen::CloseFileHandle(inHandle);
        sharpen::CloseFileHandle(outHandle);
        (void)rethrow;
        throw;
    }
    try
    {
        out = std::make_shared<sharpen::OutputPipeChannelImpl>(outHandle);
    }
    catch (const std::exception &rethrow)
    {
        sharpen::CloseFileHandle(outHandle);
        throw;
        (void)rethrow;
    }
}